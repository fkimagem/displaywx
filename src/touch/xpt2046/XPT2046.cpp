#include "XPT2046.h"

int XPT2046::XPT2046_SPISPEED = 2500000;
int XPT2046::DEFAULT_SPISPEED = 2500000;

XPT2046::XPT2046(uint8_t cs)
{
    _clk = 0;
    _miso = 0;
    _mosi = 0;
    _cs = cs;
}


uint8_t XPT2046::getInput()
{

    if (!configured)
    {
#if defined(LOGXPT)
        Serial.println("SPI and CS PIN not configured for touch XPT2046");
#endif
        return 0;
    }
    else
    {
#if defined(LOGXPT)
        //Serial.print("Reading XPT\t");
#endif
    }

    int16_t tz = 0xFFF;

    beginTouchCommunication();
    touchspi->transfer(0xb0);               // Start new Z1 conversion
    tz += touchspi->transfer16(0xc0) >> 3;  // Read Z1 and start Z2 conversion
    tz -= touchspi->transfer16(0x00) >> 3;  // Read Z2
    endTouchCommunication();
    if (tz >= 4095) tz = 0;

    z = tz;


    uint16_t tmp;
    beginTouchCommunication();

    // Start YP sample request for x position, read 4 times and keep last sample
    touchspi->transfer(0xd0);                    // Start new YP conversion
    touchspi->transfer(0);                       // Read first 8 bits
    touchspi->transfer(0xd0);                    // Read last 8 bits and start new YP conversion
    touchspi->transfer(0);                       // Read first 8 bits
    touchspi->transfer(0xd0);                    // Read last 8 bits and start new YP conversion
    touchspi->transfer(0);                       // Read first 8 bits
    touchspi->transfer(0xd0);                    // Read last 8 bits and start new YP conversion

    tmp = touchspi->transfer(0);                   // Read first 8 bits
    tmp = tmp <<5;
    tmp |= 0x1f & (touchspi->transfer(0x90)>>3);   // Read last 8 bits and start new XP conversion

    x = tmp;

    // Start XP sample request for y position, read 4 times and keep last sample
    touchspi->transfer(0);                       // Read first 8 bits
    touchspi->transfer(0x90);                    // Read last 8 bits and start new XP conversion
    touchspi->transfer(0);                       // Read first 8 bits
    touchspi->transfer(0x90);                    // Read last 8 bits and start new XP conversion
    touchspi->transfer(0);                       // Read first 8 bits
    touchspi->transfer(0x90);                    // Read last 8 bits and start new XP conversion

    tmp = touchspi->transfer(0);                 // Read first 8 bits
    tmp = tmp <<5;
    tmp |= 0x1f & (touchspi->transfer(0)>>3);    // Read last 8 bits

    y = tmp;


    endTouchCommunication();

#if defined(LOGXPT)
    //Serial.println("OK");
#endif

    if(z > XPT2046::XPT2046_ZLIMIT){
        //Serial.printf("TouchBodmer %i\t%i\t%i\n", x,y,z);
    }

    return z > XPT2046::XPT2046_ZLIMIT;
}


void XPT2046::beginTouchCommunication()
{
    touchspi->beginTransaction(SPISettings(XPT2046::XPT2046_SPISPEED, MSBFIRST, SPI_MODE0));
    digitalWrite(_cs, LOW);
}

void XPT2046::endTouchCommunication()
{
    digitalWrite(_cs, HIGH);
    touchspi->endTransaction();
}

void XPT2046::begin(SPIClass* sharedSPI, int spispeed, int defaultspispeed)
{
    bool newSPI = (sharedSPI == nullptr);
    if(defaultspispeed > 0){
        XPT2046::DEFAULT_SPISPEED = defaultspispeed;
    }
    XPT2046::XPT2046_SPISPEED = spispeed;

    if (sharedSPI)
    {
        touchspi = sharedSPI;
#if defined(LOGXPT)
        Serial.println("Using shared SPI");
#endif
    }
    else
    {
        touchspi = new SPIClass();
#if defined(LOGXPT)
        Serial.println("Using new SPI");
#endif
    }

    if (newSPI)
    {
#if defined(LOGXPT)
        Serial.println("Configuring new SPI");
#endif
        if (_clk > 0 && _miso > 0 && _mosi > 0)
        {
            touchspi->begin(_clk, _miso, _mosi);
#if defined(LOGXPT)
            Serial.println("Started XPT2046 with custom pins");
#endif
        }
        else
        {
            touchspi->begin();
#if defined(LOGXPT)
            Serial.println("Started XPT2046 with default pins");
#endif
        }
        touchspi->setFrequency(XPT2046::XPT2046_SPISPEED);
        touchspi->setDataMode(SPI_MODE0);
        touchspi->setBitOrder(MSBFIRST);
    }
    else
    {
#if defined(LOGXPT)
        Serial.println("Using shared SPI. Dont need configure");
        Serial.printf("Using pins %d %d %d %d\n", _clk, _miso, _mosi, _cs);
#endif
    }

    if (_cs > 0)
    {
        pinMode(_cs, OUTPUT);
        digitalWrite(_cs, HIGH);
        configured = true;

        getInput();
    }
}


XPT2046::XPT2046(uint8_t clk, uint8_t miso, uint8_t mosi, uint8_t cs)
{
    _clk = clk;
    _miso = miso;
    _mosi = mosi;
    _cs = cs;
}


XPT2046::~XPT2046()
{
    delete touchspi;
    touchspi = nullptr;
}

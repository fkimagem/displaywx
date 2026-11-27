#ifndef _XPT2046_H_
#define _XPT2046_H_
#include <SPI.h>
#include <Arduino.h>


// Define LOGXPT in the project configuration to enable serial diagnostics.

/// @brief Represents an interface to the XPT2046 touch controller, providing methods to initialize and read touch input.
class XPT2046 {
public:

static int XPT2046_SPISPEED;  // 2.5 MHz
static int DEFAULT_SPISPEED;  // 2.5 MHz

static const int XPT2046_ZLIMIT = 350;  // Z limit

  /// @brief Constructs an XPT2046 object with only the chip select (CS) pin specified.
  /// @param cs Chip select pin for SPI communication.
  XPT2046(uint8_t cs);

  /// @brief Constructs an XPT2046 object with full SPI pin configuration.
  /// @param clk Clock pin for SPI communication.
  /// @param miso MISO pin for SPI communication.
  /// @param mosi MOSI pin for SPI communication.
  /// @param cs Chip select pin for SPI communication.
  XPT2046(uint8_t clk, uint8_t miso, uint8_t mosi, uint8_t cs);

  /// @brief Destructor for the XPT2046 class.
  ~XPT2046();
  XPT2046(const XPT2046&) = delete;
  XPT2046& operator=(const XPT2046&) = delete;
  void end();

  /// @brief Initializes the XPT2046 with a shared SPI instance.
  /// @param sharedSPI Pointer to the shared SPI instance.
  /// @param spispeed SPI speed in Hz for Touch CI.
  /// @param defaultspispeed Default SPI speed in Hz for shared SPI.
  void begin(SPIClass* sharedSPI, int spispeed, int defaultspispeed);

  /// @brief Reads touch input values (x, y, and z coordinates).
  /// @return A status byte indicating the result of the touch input read.
  uint8_t getInput();

  /// @brief Start transaction and turn CS pin LOW
  void beginTouchCommunication();

  /// @brief Finish transaction and turn CS pin HIGH
  void endTouchCommunication();

  uint8_t _cs; ///< Chip select pin.
  uint8_t _clk; ///< Clock pin.
  uint8_t _miso; ///< MISO pin.
  uint8_t _mosi; ///< MOSI pin.

  uint16_t x; ///< X-coordinate of the last touch.
  uint16_t y; ///< Y-coordinate of the last touch.
  uint16_t z; ///< Pressure (Z) value of the last touch.

  bool configured = false; ///< Flag indicating if the XPT2046 has been configured.

  SPIClass *touchspi = nullptr; ///< Pointer to the SPI instance used for touch input.
private:
  bool m_ownsSpi = false;
};


#endif

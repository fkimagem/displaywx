#include <displaywx.h>

// SD_CS, SD_MISO, SD_MOSI e SD_SCK pertencem ao sketch da placa.
// Se o gerador publicar SD_CLK, o proprio sketch cria o alias antes desta chamada:
// #define SD_SCK SD_CLK

DisplayWX myDisplay;

void configurarSD()
{
#if defined(DFK_SD) && defined(SD_CS) && defined(SD_MISO) && defined(SD_MOSI) && defined(SD_SCK)
    SDConfig config;
    config.cs = SD_CS;
    config.sck = SD_SCK;
    config.miso = SD_MISO;
    config.mosi = SD_MOSI;
    config.spi = &SPI;
#if defined(SD_FREQUENCY)
    config.frequencyHz = SD_FREQUENCY;
#endif
    if (!myDisplay.startSD(config))
    {
        Serial.println("SD indisponivel; interface continua ativa.");
        return;
    }

    if (!myDisplay.writeSDFile("/config.txt", "modo=automatico\n"))
    {
        Serial.println("Falha ao gravar configuracao.");
    }
    if (!myDisplay.appendSDFile("/eventos.txt", "Sistema iniciado\n"))
    {
        Serial.println("Falha ao acrescentar evento.");
    }

    const uint8_t dados[] = {0x01, 0x00, 0xFE, 0x02};
    uint8_t retorno[4] = {};
    size_t lidos = 0;
    const bool gravou = myDisplay.writeSDFile("/dados.bin", dados, sizeof(dados));
    const bool leu = myDisplay.readSDFile("/dados.bin", retorno, sizeof(retorno), lidos);
    if (!gravou || !leu || lidos != sizeof(dados) || memcmp(dados, retorno, sizeof(dados)) != 0)
    {
        Serial.println("Falha na comparacao binaria.");
    }

    String conteudo;
    if (myDisplay.readSDFile("/config.txt", conteudo))
    {
        Serial.print(conteudo);
    }
#else
    Serial.println("Pinos SD ausentes; display e touch seguem sem cartao.");
#endif
}

void setup()
{
    Serial.begin(115200);
    configurarSD();
}

void loop() {}

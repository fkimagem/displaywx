#include <displaywx.h>

// Formato DisplayWX binario v1, RGB565, 2 x 1, vermelho e verde, ambos visiveis:
// 00 02 00 01 F8 00 07 E0 00 01 C0
// A mascara e obrigatoria: 1 bit por pixel, MSB primeiro, linhas alinhadas a byte.

DisplayWX myDisplay;
#if defined(DFK_IMAGE)
Image logo(10, 10, 0);
Image *imagens[] = {&logo};
#endif

#if defined(DFK_SD)
bool gravarLogo()
{
    if (!myDisplay.createSDDir("/imagens"))
    {
        return false;
    }
    const uint8_t logoBin[] = {0x00, 0x02, 0x00, 0x01, 0xF8, 0x00, 0x07, 0xE0, 0x00, 0x01, 0xC0};
    return myDisplay.writeSDFile("/imagens/logo.bin", logoBin, sizeof(logoBin));
}
#endif

void configurarLogo()
{
#if defined(DFK_IMAGE) && defined(DFK_SD)
    if (!myDisplay.isSDReady())
    {
        Serial.println("Logo indisponivel; usar interface sem imagem ou pixels embutidos.");
        return;
    }
    if (!gravarLogo())
    {
        Serial.println("Falha ao gravar a imagem de exemplo.");
        return;
    }

    ImageFromFileConfig config;
    config.path = "/imagens/logo.bin";
    config.cb = nullptr;
    config.source = SourceFile::SD;
    config.backgroundColor = CFK_BLACK;
    if (logo.setupFromFileSafe(config))
    {
        myDisplay.setImage(imagens, 1);
        return;
    }
#endif
    Serial.println("Logo indisponivel; usar interface sem imagem ou pixels embutidos.");
}

void setup()
{
    Serial.begin(115200);
    // O objeto de desenho e a montagem do cartao acontecem antes desta carga,
    // e antes de createTask(). Sem cartao ou sem display, a carga retorna false.
    configurarLogo();
}

void loop() {}

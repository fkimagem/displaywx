#include <SPI.h>
#include <Arduino_GFX_Library.h>
#include <displaywx.h>

// ESP32-S3 com a placa 4827S043. O display é RGB.
// O cartão usa o mesmo SPI do XPT2046; o CS do SD é o pino 10.

const int DISPLAY_W = 480;
const int DISPLAY_H = 272;
const int TOUCH_FREQUENCY = 2500000;
const int TOUCH_CS = 38;
const int TOUCH_SCK = 12;
const int TOUCH_MOSI = 11;
const int TOUCH_MISO = 13;
const int SD_CS = 10;
const int DISP_LED = 2;
const uint8_t rotationScreen = 0;
const bool isIPS = false;

const int kMargem = 8;
const int kRecuoPorNivel = 10;
const int kAlturaLinha = 12;
const int kLadoMarcador = 8;
const int kFolgaMarcador = 4;
const int kIconeLado = 32;
const int kIconeX = DISPLAY_W - kMargem - kIconeLado;
const int kIconeY = 8;
const uint8_t kMaxEntradas = 40;
const uint8_t kMaxNivel = 8;

const bool TOUCH_INVERT_X = false;
const bool TOUCH_INVERT_Y = false;
const int TOUCH_MAP_X0 = 0;
const int TOUCH_MAP_X1 = 480;
const int TOUCH_MAP_Y0 = 0;
const int TOUCH_MAP_Y1 = 272;
const bool TOUCH_SWAP_XY = false;

struct EntradaSd {
    char nome[48];
    uint8_t nivel;
    bool pasta;
};

#if defined(CONFIG_IDF_TARGET_ESP32S3)
SPIClass spi_shared(FSPI);
#else
SPIClass spi_shared(HSPI);
#endif
Arduino_ESP32RGBPanel *bus = nullptr;
Arduino_RGB_Display *tft = nullptr;
DisplayWX myDisplay;
Image icone(kIconeX, kIconeY, 0);
Image *imagens[] = {&icone};
bool iconeCarregado = false;
EntradaSd entradas[kMaxEntradas];
uint8_t quantidadeEntradas = 0;

void deixarSelecoesInativas()
{
    pinMode(TOUCH_CS, OUTPUT);
    pinMode(SD_CS, OUTPUT);
    digitalWrite(TOUCH_CS, HIGH);
    digitalWrite(SD_CS, HIGH);
}

void copiarNome(char *destino, size_t capacidade, const char *origem)
{
    if (destino == nullptr || capacidade == 0)
    {
        return;
    }
    if (origem == nullptr || origem[0] == '\0')
    {
        destino[0] = '\0';
        return;
    }
    const char *exibicao = origem;
    for (const char *cursor = origem; *cursor != '\0'; ++cursor)
    {
        if (*cursor == '/')
        {
            exibicao = cursor + 1;
        }
    }
    if (exibicao[0] == '\0')
    {
        exibicao = origem;
    }
    strncpy(destino, exibicao, capacidade - 1);
    destino[capacidade - 1] = '\0';
}

bool montarCaminho(const char *pai, const char *caminhoItem, const char *nome, char *destino, size_t capacidade)
{
    const char *origem = caminhoItem;
    if (origem == nullptr || origem[0] == '\0')
    {
        origem = nome;
    }
    if (origem != nullptr && origem[0] == '/')
    {
        strncpy(destino, origem, capacidade - 1);
        destino[capacidade - 1] = '\0';
        return destino[0] != '\0';
    }
    if (pai == nullptr || origem == nullptr || origem[0] == '\0')
    {
        return false;
    }
    const size_t tamanhoPai = strlen(pai);
    const bool paiTemBarra = tamanhoPai > 0 && pai[tamanhoPai - 1] == '/';
    const size_t necessario = tamanhoPai + (paiTemBarra ? 0u : 1u) + strlen(origem) + 1u;
    if (necessario > capacidade)
    {
        return false;
    }
    memcpy(destino, pai, tamanhoPai);
    size_t cursor = tamanhoPai;
    if (!paiTemBarra)
    {
        destino[cursor++] = '/';
    }
    memcpy(destino + cursor, origem, strlen(origem) + 1);
    return true;
}

void coletarDiretorio(fs::FS &cartao, const char *caminho, uint8_t nivel)
{
    if (nivel > kMaxNivel || quantidadeEntradas >= kMaxEntradas)
    {
        return;
    }

    fs::File pasta = cartao.open(caminho);
    if (!pasta || !pasta.isDirectory())
    {
        if (pasta)
        {
            pasta.close();
        }
        return;
    }

    fs::File item = pasta.openNextFile();
    while (item)
    {
        const char *nome = item.name();
        const char *caminhoItem = item.path();
        char caminhoFilho[96];
        const bool temCaminho = montarCaminho(caminho, caminhoItem, nome, caminhoFilho, sizeof(caminhoFilho));
        const bool pastaItem = item.isDirectory();
        char nomeVisivel[48];
        copiarNome(nomeVisivel, sizeof(nomeVisivel), nome);
        item.close();

        const bool oculto = strcmp(nomeVisivel, ".") == 0 || strcmp(nomeVisivel, "..") == 0;
        if (!oculto && nomeVisivel[0] != '\0' && quantidadeEntradas < kMaxEntradas)
        {
            strncpy(entradas[quantidadeEntradas].nome, nomeVisivel, sizeof(entradas[quantidadeEntradas].nome) - 1);
            entradas[quantidadeEntradas].nome[sizeof(entradas[quantidadeEntradas].nome) - 1] = '\0';
            entradas[quantidadeEntradas].nivel = nivel;
            entradas[quantidadeEntradas].pasta = pastaItem;
            ++quantidadeEntradas;
        }

        if (pastaItem && temCaminho && !oculto)
        {
            coletarDiretorio(cartao, caminhoFilho, static_cast<uint8_t>(nivel + 1));
        }
        item = pasta.openNextFile();
    }
    pasta.close();
}

void desenharNome(const char *texto, int x, int y)
{
    tft->setFont();
    tft->setTextSize(1);
    tft->setTextColor(CFK_BLACK);
    tft->setCursor(x, y);
    tft->print(texto);
}

void desenharMarcador(int y, bool pasta)
{
    const int topo = y + (kAlturaLinha - kLadoMarcador) / 2;
    tft->fillRect(kMargem, topo, kLadoMarcador, kLadoMarcador, pasta ? CFK_YELLOW : CFK_LIME);
}

bool carregarIcone()
{
    if (!myDisplay.isSDReady())
    {
        return false;
    }

    ImageFromFileConfig config;
    config.path = "/icone.fki";
    config.cb = nullptr;
    config.source = SourceFile::SD;
    config.backgroundColor = CFK_WHITE;
    if (!icone.setupFromFileSafe(config))
    {
        return false;
    }

    myDisplay.setImage(imagens, 1);
    return true;
}

void desenharIcone()
{
    if (!iconeCarregado)
    {
        desenharNome("sem icone", kIconeX, kIconeY);
        return;
    }

    icone.forceUpdate();
    icone.draw();
}

void desenharTela()
{
    tft->fillScreen(CFK_WHITE);
    myDisplay.setBackgroundColor(CFK_WHITE);
    myDisplay.printText("teste de sd", DISPLAY_W / 2, 8, TC_DATUM, CFK_BLACK, CFK_WHITE, &RobotoRegular10pt7b);

    int y = 40;
    if (!myDisplay.isSDReady())
    {
        desenharNome("cartao ausente", kMargem + kLadoMarcador + kFolgaMarcador, y);
    }
    else if (quantidadeEntradas == 0)
    {
        desenharNome("cartao vazio", kMargem + kLadoMarcador + kFolgaMarcador, y);
    }
    else
    {
        for (uint8_t indice = 0; indice < quantidadeEntradas; ++indice)
        {
            if (y + kAlturaLinha > DISPLAY_H)
            {
                break;
            }
            const int x = kMargem + kLadoMarcador + kFolgaMarcador + (entradas[indice].nivel * kRecuoPorNivel);
            desenharMarcador(y, entradas[indice].pasta);
            desenharNome(entradas[indice].nome, x, y);
            y += kAlturaLinha;
        }
    }

    desenharIcone();
}

void setup()
{
    Serial.begin(115200);
    deixarSelecoesInativas();
    spi_shared.begin(TOUCH_SCK, TOUCH_MISO, TOUCH_MOSI);

    bus = new Arduino_ESP32RGBPanel(
        40, 41, 39, 42,
        45, 48, 47, 21, 14,
        5, 6, 7, 15, 16, 4,
        8, 3, 46, 9, 1,
        0, 1, 1, 43,
        0, 3, 1, 12,
        1, 10000000, isIPS,
        0, 0, 0);
    tft = new Arduino_RGB_Display(DISPLAY_W, DISPLAY_H, bus, rotationScreen, true);
    tft->begin();
    pinMode(DISP_LED, OUTPUT);
    digitalWrite(DISP_LED, HIGH);
    myDisplay.setDrawObject(tft);
#if defined(TOUCH_XPT2046)
myDisplay.setTouchCorners(TOUCH_MAP_X0, TOUCH_MAP_X1, TOUCH_MAP_Y0, TOUCH_MAP_Y1);
    myDisplay.setInvertAxis(TOUCH_INVERT_X, TOUCH_INVERT_Y);
    myDisplay.setSwapAxis(TOUCH_SWAP_XY);
    myDisplay.startTouchXPT2046(DISPLAY_W, DISPLAY_H, rotationScreen, TOUCH_CS, &spi_shared, tft, TOUCH_FREQUENCY);
    myDisplay.enableTouchLog();
    myDisplay.recalibrate();
    myDisplay.checkCalibration();
#endif

    deixarSelecoesInativas();
    SDConfig config;
    config.cs = SD_CS;
    config.spi = &spi_shared;
    config.initializeSPI = false;
    config.frequencyHz = 4000000;
    if (myDisplay.startSD(config))
    {
        fs::FS *cartao = myDisplay.getSD();
        if (cartao != nullptr)
        {
            coletarDiretorio(*cartao, "/", 0);
        }
    }

    iconeCarregado = carregarIcone();
    //desenharTela();
    myDisplay.loadScreen(desenharTela); // Use this line to change between screens
    myDisplay.createTask(false, 3); // Initialize the task to read touch and draw
}

void loop() {}

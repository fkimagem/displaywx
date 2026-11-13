/*
  AURA - controle de ambiente SIMULADO.
  Placa: ESP32S3; display ST7796 320 x 480; touch FT6336U.
  Em user_setup.h da biblioteca: DISP_DEFAULT e TOUCH_FT6336.
  Nao ha sensores, Wi-Fi, motor ou rele: os valores sao simulados pelo programa.
  Veja README.md para aprender a mudar cores, tamanhos, escalas e perfis.
*/
#include <SPI.h>
#include <Arduino_GFX_Library.h>
#include <displaywx.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>

// 1. PERSONALIZE AQUI: escalas, comportamento e perfis.
const int TEMP_ESCALA_MIN = 10;  // Escala do anel, em graus C.
const int TEMP_ESCALA_MAX = 40;
const int AJUSTE_MIN = 16;       // Limites dos botoes - e +.
const int AJUSTE_MAX = 30;
const int AJUSTE_PASSO = 1;
const int VENT_ESCALA_MAX = 100; // Escala do slider; a simulacao usa 0 a 100%.
const unsigned long INTERVALO_SIMULACAO_MS = 500;
constexpr float VELOCIDADE_SIMULACAO = 1.0f;
const float TEMP_INICIAL = 28.0f;
const float TEMP_EXTERNA = 30.0f;
const float UMIDADE_INICIAL = 58.0f;
const float QUALIDADE_INICIAL = 70.0f; // Indice ficticio: maior = melhor.
const int META_ECO = 26;
const int META_CONFORTO = 24;
const int META_TURBO = 20;
const int LIMITE_ECO = 40;       // Ventilacao maxima de cada perfil, em %.
const int LIMITE_CONFORTO = 70;
const int LIMITE_TURBO = 100;
const float FAIXA_ESTAVEL = 0.3f;

static_assert(TEMP_ESCALA_MIN < TEMP_ESCALA_MAX, "Confira a escala do anel");
static_assert(AJUSTE_MIN < AJUSTE_MAX, "Confira os limites do ajuste");
static_assert(AJUSTE_PASSO > 0, "O passo precisa ser positivo");
static_assert(VENT_ESCALA_MAX > 0 && VENT_ESCALA_MAX <= 1000, "Use escala de ventilacao entre 1 e 1000");
static_assert(INTERVALO_SIMULACAO_MS >= 50, "Use intervalo de pelo menos 50 ms");
static_assert(VELOCIDADE_SIMULACAO > 0 && VELOCIDADE_SIMULACAO <= 5, "Use velocidade entre 0 e 5, sem zero");

// 2. PERSONALIZE AQUI: cores e letras.
const uint16_t COR_FUNDO = CFK_BLACK;
const uint16_t COR_TEXTO = CFK_WHITE;
const uint16_t COR_LEGENDA = CFK_GREY6;
const uint16_t COR_PAINEL = CFK_GREY3;
const uint16_t COR_ANEL = CFK_COLOR043;
const uint16_t COR_ANEL_VAZIO = CFK_GREY6;
const uint16_t COR_VENTILACAO = CFK_COLOR062;
const uint16_t COR_TRILHA = CFK_WHITE;
const uint16_t COR_DADOS = CFK_COLOR038;
const uint16_t COR_AUTO = CFK_COLOR032;
const uint16_t COR_ECO_FUNDO = CFK_GREY3;
const uint16_t COR_ECO_TEXTO = CFK_COLOR025;
const uint16_t COR_CONFORTO_FUNDO = CFK_COLOR044;
const uint16_t COR_TURBO_FUNDO = CFK_COLOR008;
const uint16_t COR_BOTAO_TEXTO = CFK_BLACK;
const GFXfont* const FONTE_UMIDADE = &RobotoRegular15pt7b;

// 3. PERSONALIZE AQUI: posicoes e tamanhos, em pixels.
const int ANEL_X = 80;          // Centro do anel.
const int ANEL_Y = 160;
const int ANEL_RAIO = 60;
const int ANEL_ESPESSURA = 20;
const int SLIDER_X = 28;        // Canto superior esquerdo.
const int SLIDER_Y = 311;
const int SLIDER_LARGURA = 271;
const int SLIDER_RAIO = 13;     // Altura = 2 vezes o raio.
const int BOTAO_LARGURA = 89;
const int BOTAO_ALTURA = 35;
const int BOTAO_CANTO = 10;
static_assert(ANEL_RAIO - ANEL_ESPESSURA >= 15, "Deixe espaco para o numero do anel");
static_assert(SLIDER_RAIO > 4 && SLIDER_LARGURA > 2 * SLIDER_RAIO, "Confira o tamanho do slider");

// 4. LIGACOES: pinos originais do display e do touch, preservados.
const int DISPLAY_W = 320;
const int DISPLAY_H = 480;
const int DISP_FREQUENCY = 27000000;
const int TOUCH_MAP_X0 = 0;
const int TOUCH_MAP_Y0 = 0;
const int TOUCH_MAP_X1 = 320;
const int TOUCH_MAP_Y1 = 480;
const bool TOUCH_SWAP_XY = false;
const bool TOUCH_INVERT_X = false;
const bool TOUCH_INVERT_Y = false;
const int DISP_MOSI = 11;
const int DISP_MISO = -1;
const int DISP_SCLK = 12;
const int DISP_CS = 10;
const int DISP_DC = 8;
const int DISP_RST = 9;
const int TOUCH_SCL = 14;
const int TOUCH_SDA = 13;
const int TOUCH_INT = 18;
const int TOUCH_RST = 17;
const uint8_t rotationScreen = 0;
const bool isIPS = true;

// 5. MEMORIA DA SIMULACAO. Somente o loop modifica este estado.
enum class Perfil { ECO, CONFORTO, TURBO, PERSONALIZADO };
enum class TipoComando { VENTILACAO, AJUSTE, AUTO, ECO, CONFORTO, TURBO };
struct Comando {
    TipoComando tipo;
    int valor;
};
struct EstadoAmbiente {
    float temperatura;
    float umidade;
    float qualidade;
    int meta;
    int ventilacao; // Sempre percentual: 0 a 100.
    int limiteAutomatico;
    bool automatico;
    Perfil perfil;
};
EstadoAmbiente ambiente = {
    TEMP_INICIAL, UMIDADE_INICIAL, QUALIDADE_INICIAL,
    META_CONFORTO, 0, LIMITE_CONFORTO, true, Perfil::CONFORTO
};
QueueHandle_t filaComandos = nullptr;
unsigned long ultimaSimulacao = 0;

// Avisos das funcoes usadas mais adiante.
void screen0();
void loadWidgets();
void atualizarWidgets();
void enviarComando(TipoComando tipo, int valor);
void aplicarComando(EstadoAmbiente& estado, const Comando& comando);
void aplicarPerfil(EstadoAmbiente& estado, Perfil perfil);
void ajustarVentilacaoAutomatica(EstadoAmbiente& estado);
void simularPasso(EstadoAmbiente& estado, float segundos);
int limitarInteiro(int valor, int minimo, int maximo);
float limitarDecimal(float valor, float minimo, float maximo);
int valorSliderParaPercentual(int valor);
int percentualParaValorSlider(int percentual);
const char* nomePerfil(Perfil perfil);
const char* nomeQualidade(float qualidade);
const char* nomeAcao(const EstadoAmbiente& estado);
void slider1_cb();
void spinbox1_cb();
void toggle1_cb();
void textbutton1_cb();
void textbutton1copy_cb();
void textbutton1copy2_cb();

// 6. OBJETOS: cada nome representa uma peca da tela.
SPIClass spiPrimary(HSPI);
Arduino_DataBus* bus = nullptr;
Arduino_GFX* tft = nullptr;
DisplayWX myDisplay;
HSlider slider1(SLIDER_X, SLIDER_Y, 0);
Led led1(30, 465, 0);
SpinBox spinbox1(195, 15, 0);
TextButton textbutton1(15, 410, 0);
TextButton textbutton1copy(115, 410, 0);
TextButton textbutton1copy2(220, 410, 0);
ToggleButton toggle1(230, 370, 0);
CircularBar circload1(ANEL_X, ANEL_Y, 0);
Label textoUmidade(180, 133, 0);
Label textoQualidade(180, 213, 0);
Label textoVentilacao(295, 281, 0); // Alinhado pela direita.
Label textoEstado(50, 461, 0);
HSlider* arrayHslider[] = {&slider1};
Led* arrayLed[] = {&led1};
SpinBox* arraySpinbox[] = {&spinbox1};
TextButton* arrayTextButton[] = {&textbutton1, &textbutton1copy, &textbutton1copy2};
ToggleButton* arrayTogglebtn[] = {&toggle1};
CircularBar* arrayCircularbar[] = {&circload1};
Label* arrayLabel[] = {&textoUmidade, &textoQualidade, &textoVentilacao, &textoEstado};

// 7. REGRAS DA SIMULACAO: nao acessam sensores nem desenham na tela.
int limitarInteiro(int valor, int minimo, int maximo) {
    if (valor < minimo) return minimo;
    if (valor > maximo) return maximo;
    return valor;
}
float limitarDecimal(float valor, float minimo, float maximo) {
    if (valor < minimo) return minimo;
    if (valor > maximo) return maximo;
    return valor;
}
int valorSliderParaPercentual(int valor) {
    valor = limitarInteiro(valor, 0, VENT_ESCALA_MAX);
    return (valor * 100 + VENT_ESCALA_MAX / 2) / VENT_ESCALA_MAX;
}
int percentualParaValorSlider(int percentual) {
    return (limitarInteiro(percentual, 0, 100) * VENT_ESCALA_MAX + 50) / 100;
}
void ajustarVentilacaoAutomatica(EstadoAmbiente& estado) {
    if (!estado.automatico) return;
    float erro = estado.temperatura - estado.meta;
    if (erro < 0) erro = -erro; // Distancia ate a meta, sem sinal negativo.
    int pedido = (erro <= FAIXA_ESTAVEL) ? 20 : 35 + int(erro * 18);
    estado.ventilacao = limitarInteiro(pedido, 0, estado.limiteAutomatico);
}
void aplicarPerfil(EstadoAmbiente& estado, Perfil perfil) {
    estado.perfil = perfil;
    estado.automatico = true;
    if (perfil == Perfil::ECO) {
        estado.meta = META_ECO;
        estado.limiteAutomatico = LIMITE_ECO;
    } else if (perfil == Perfil::CONFORTO) {
        estado.meta = META_CONFORTO;
        estado.limiteAutomatico = LIMITE_CONFORTO;
    } else {
        estado.meta = META_TURBO;
        estado.limiteAutomatico = LIMITE_TURBO;
    }
    estado.meta = limitarInteiro(estado.meta, AJUSTE_MIN, AJUSTE_MAX);
    estado.limiteAutomatico = limitarInteiro(estado.limiteAutomatico, 0, 100);
    ajustarVentilacaoAutomatica(estado);
}
void aplicarComando(EstadoAmbiente& estado, const Comando& comando) {
    switch (comando.tipo) {
        case TipoComando::VENTILACAO:
            estado.ventilacao = valorSliderParaPercentual(comando.valor);
            estado.automatico = false; // Arrastar o slider assume o controle manual.
            estado.perfil = Perfil::PERSONALIZADO;
            estado.limiteAutomatico = 100;
            break;
        case TipoComando::AJUSTE:
            estado.meta = limitarInteiro(comando.valor, AJUSTE_MIN, AJUSTE_MAX);
            estado.perfil = Perfil::PERSONALIZADO;
            estado.limiteAutomatico = 100;
            break;
        case TipoComando::AUTO:
            estado.automatico = comando.valor != 0;
            break;
        case TipoComando::ECO: aplicarPerfil(estado, Perfil::ECO); break;
        case TipoComando::CONFORTO: aplicarPerfil(estado, Perfil::CONFORTO); break;
        case TipoComando::TURBO: aplicarPerfil(estado, Perfil::TURBO); break;
    }
    ajustarVentilacaoAutomatica(estado);
}
void simularPasso(EstadoAmbiente& estado, float segundos) {
    // Passos pequenos mantem a animacao suave, mesmo apos uma pausa.
    segundos = limitarDecimal(segundos, 0.0f, 2.0f);
    ajustarVentilacaoAutomatica(estado);
    float forca = estado.ventilacao / 100.0f;
    if (estado.automatico) {
        // A sala tende lentamente para a temperatura externa.
        estado.temperatura += (TEMP_EXTERNA - estado.temperatura) * 0.02f * segundos;
        float erro = estado.meta - estado.temperatura;
        float passo = 0.6f * forca * segundos;
        if (erro > FAIXA_ESTAVEL) estado.temperatura += (erro < passo ? erro : passo);
        if (erro < -FAIXA_ESTAVEL) estado.temperatura -= (-erro < passo ? -erro : passo);
    } else {
        // Nesta simulacao, a ventilacao pode reduzir a temperatura em ate 6 graus.
        float destino = TEMP_EXTERNA - 6.0f * forca;
        estado.temperatura += (destino - estado.temperatura) * 0.08f * segundos;
    }
    // Mais ventilacao tende a reduzir a umidade e melhorar o indice de ar.
    float umidadeDestino = 70.0f - 35.0f * forca;
    float qualidadeDestino = 20.0f + 80.0f * forca;
    estado.umidade += (umidadeDestino - estado.umidade) * 0.12f * segundos;
    estado.qualidade += (qualidadeDestino - estado.qualidade) * 0.18f * segundos;
    estado.temperatura = limitarDecimal(estado.temperatura, 5.0f, 45.0f);
    estado.umidade = limitarDecimal(estado.umidade, 0.0f, 100.0f);
    estado.qualidade = limitarDecimal(estado.qualidade, 0.0f, 100.0f);
}
const char* nomePerfil(Perfil perfil) {
    switch (perfil) {
        case Perfil::ECO: return "ECO";
        case Perfil::CONFORTO: return "CONFORTO";
        case Perfil::TURBO: return "TURBO";
        default: return "PERSONAL";
    }
}
const char* nomeQualidade(float qualidade) {
    if (qualidade >= 70.0f) return "BOM";
    if (qualidade >= 40.0f) return "REGULAR";
    return "RUIM";
}
const char* nomeAcao(const EstadoAmbiente& estado) {
    if (!estado.automatico) return estado.ventilacao > 0 ? "VENTILANDO" : "PARADO";
    if (estado.ventilacao == 0) return "SEM VENTILACAO";
    if (estado.temperatura > estado.meta + FAIXA_ESTAVEL) return "RESFRIANDO";
    if (estado.temperatura < estado.meta - FAIXA_ESTAVEL) return "AQUECENDO";
    return "ESTAVEL";
}

// 8. INICIO E REPETICAO.
void setup() {
    Serial.begin(115200);
    filaComandos = xQueueCreate(16, sizeof(Comando));
    if (filaComandos == nullptr) {
        Serial.println("Nao foi possivel criar a fila de comandos.");
        return;
    }
    spiPrimary.begin(DISP_SCLK, DISP_MISO, DISP_MOSI);
    bus = new Arduino_HWSPI(DISP_DC, DISP_CS, DISP_SCLK, DISP_MOSI, DISP_MISO, &spiPrimary);
    tft = new Arduino_ST7796(bus, DISP_RST, rotationScreen, isIPS, 320, 480);
    tft->begin(DISP_FREQUENCY);
    myDisplay.setDrawObject(tft);
    myDisplay.setTouchCorners(TOUCH_MAP_X0, TOUCH_MAP_X1, TOUCH_MAP_Y0, TOUCH_MAP_Y1);
    myDisplay.setInvertAxis(TOUCH_INVERT_X, TOUCH_INVERT_Y);
    myDisplay.setSwapAxis(TOUCH_SWAP_XY);
    myDisplay.startTouchFT6336(DISPLAY_W, DISPLAY_H, rotationScreen, TOUCH_SDA, TOUCH_SCL, TOUCH_INT, TOUCH_RST);
    loadWidgets();
    aplicarPerfil(ambiente, Perfil::CONFORTO);
    atualizarWidgets();
    myDisplay.loadScreen(screen0);
    ultimaSimulacao = millis();
    myDisplay.createTask(false, 3);
    Serial.println("AURA: simulacao iniciada. Nao ha sensores nem atuadores reais.");
}
void loop() {
    if (filaComandos == nullptr) { delay(100); return; }
    bool mudou = false;
    Comando comando;
    // Cada comando da fila representa uma acao feita na tela.
    for (int i = 0; i < 16 && xQueueReceive(filaComandos, &comando, 0) == pdTRUE; ++i) {
        aplicarComando(ambiente, comando);
        mudou = true;
    }
    unsigned long agora = millis();
    unsigned long decorrido = agora - ultimaSimulacao;
    if (decorrido >= INTERVALO_SIMULACAO_MS) {
        ultimaSimulacao = agora;
        float segundos = decorrido / 1000.0f * VELOCIDADE_SIMULACAO;
        simularPasso(ambiente, segundos);
        mudou = true;
    }
    if (mudou) atualizarWidgets();
    delay(10); // Pausa curta: a interface continua respondendo.
}
void atualizarWidgets() {
    myDisplay.startCustomDraw();
    // false atualiza o desenho sem enviar um novo comando pelo callback.
    int valorSlider = percentualParaValorSlider(ambiente.ventilacao);
    if (slider1.getValue() != valorSlider) slider1.setValue(valorSlider, false);
    if (spinbox1.getValue() != ambiente.meta) spinbox1.setValue(ambiente.meta, false);
    if (toggle1.getStatus() != ambiente.automatico) toggle1.setStatus(ambiente.automatico, false);
    circload1.setValue(int(ambiente.temperatura + 0.5f)); // Mostra inteiro arredondado.
    textoUmidade.setTextInt(int(ambiente.umidade + 0.5f));
    textoQualidade.setText(nomeQualidade(ambiente.qualidade));
    textoVentilacao.setTextInt(ambiente.ventilacao);
    led1.setState(ambiente.ventilacao > 0);
    char mensagem[64];
    if (ambiente.automatico) {
        snprintf(mensagem, sizeof(mensagem), "AUTO %s / %s", nomePerfil(ambiente.perfil), nomeAcao(ambiente));
    } else {
        snprintf(mensagem, sizeof(mensagem), "MANUAL / %s", nomeAcao(ambiente));
    }
    textoEstado.setText(mensagem);
    myDisplay.finishCustomDraw();
}

// 9. DESENHO FIXO. Os valores variaveis sao os Labels registrados abaixo.
void screen0() {
    myDisplay.setBackgroundColor(COR_FUNDO);
    tft->fillRoundRect(12, 270, 298, 80, 20, COR_PAINEL);
    myDisplay.printText("AURA", 10, 18, TL_DATUM, COR_TEXTO, COR_FUNDO, &RobotoBold15pt7b);
    myDisplay.printText("SALA / SIMULACAO DE AMBIENTE", 10, 56, TL_DATUM, COR_TEXTO, COR_FUNDO, &RobotoRegular6pt7b);
    myDisplay.printText("AJUSTE C", 215, 61, TL_DATUM, COR_LEGENDA, COR_FUNDO, &RobotoRegular7pt7b);
    myDisplay.printText("TEMP ATUAL / C", 28, 231, TL_DATUM, COR_LEGENDA, COR_FUNDO, &RobotoRegular7pt7b);
    myDisplay.printText("UMIDADE", 180, 106, TL_DATUM, COR_LEGENDA, COR_FUNDO, &RobotoRegular7pt7b);
    myDisplay.printText("QUALIDADE DO AR", 180, 186, TL_DATUM, COR_LEGENDA, COR_FUNDO, &RobotoRegular7pt7b);
    myDisplay.printText("VENTILACAO", 30, 281, TL_DATUM, COR_TEXTO, COR_PAINEL, &RobotoBold7pt7b);
    myDisplay.printText("MODO AUTO", 35, 376, TL_DATUM, COR_TEXTO, COR_FUNDO, &RobotoRegular7pt7b);
    myDisplay.drawWidgetsOnScreen(0);
}

// 10. FICHAS DE CONFIGURACAO. setup() de cada widget e chamado uma vez.
void loadWidgets() {
    HSliderConfig configHSlider0 = {
        .callback = slider1_cb, .subtitle = nullptr,
        .minValue = 0, .maxValue = VENT_ESCALA_MAX,
        .radius = SLIDER_RAIO, .width = SLIDER_LARGURA,
        .pressedColor = COR_VENTILACAO, .backgroundColor = COR_TRILHA
    };
    slider1.setup(configHSlider0);
    myDisplay.setHSlider(arrayHslider, 1);
    LedConfig configLed0 = {
        .radius = 8, .colorOn = COR_AUTO, .colorOff = COR_PAINEL, .initialState = false
    };
    led1.setup(configLed0);
    myDisplay.setLed(arrayLed, 1);
    SpinBoxConfig configSpinBox0 = {
        .callback = spinbox1_cb, .minValue = AJUSTE_MIN, .maxValue = AJUSTE_MAX,
        .startValue = META_CONFORTO, .width = 112, .height = 40, .step = AJUSTE_PASSO,
        .color = COR_ANEL_VAZIO, .textColor = COR_ANEL
    };
    spinbox1.setup(configSpinBox0);
    myDisplay.setSpinbox(arraySpinbox, 1);
    TextButtonConfig configEco = {
        .text = "ECO", .callback = textbutton1_cb, .fontFamily = &RobotoBold10pt7b,
        .width = BOTAO_LARGURA, .height = BOTAO_ALTURA, .radius = BOTAO_CANTO,
        .backgroundColor = COR_ECO_FUNDO, .textColor = COR_ECO_TEXTO
    };
    textbutton1.setup(configEco);
    TextButtonConfig configConforto = {
        .text = "CONFORTO", .callback = textbutton1copy_cb, .fontFamily = &RobotoBold7pt7b,
        .width = BOTAO_LARGURA, .height = BOTAO_ALTURA, .radius = BOTAO_CANTO,
        .backgroundColor = COR_CONFORTO_FUNDO, .textColor = COR_BOTAO_TEXTO
    };
    textbutton1copy.setup(configConforto);
    TextButtonConfig configTurbo = {
        .text = "TURBO", .callback = textbutton1copy2_cb, .fontFamily = &RobotoBold10pt7b,
        .width = BOTAO_LARGURA, .height = BOTAO_ALTURA, .radius = BOTAO_CANTO,
        .backgroundColor = COR_TURBO_FUNDO, .textColor = COR_BOTAO_TEXTO
    };
    textbutton1copy2.setup(configTurbo);
    myDisplay.setTextButton(arrayTextButton, 3);
    ToggleButtonConfig configToggle0 = {
        .callback = toggle1_cb, .width = 40, .height = 20, .pressedColor = COR_AUTO
    };
    toggle1.setup(configToggle0);
    myDisplay.setToggle(arrayTogglebtn, 1);
    CircularBarConfig configCirculaBar0 = {
        .minValue = TEMP_ESCALA_MIN, .maxValue = TEMP_ESCALA_MAX,
        .radius = ANEL_RAIO, .startAngle = 0, .endAngle = 360,
        .color = COR_ANEL, .backgroundColor = COR_ANEL_VAZIO,
        .textColor = COR_TEXTO, .backgroundText = COR_FUNDO,
        .thickness = ANEL_ESPESSURA, .showValue = true, .inverted = false
    };
    circload1.setup(configCirculaBar0);
    myDisplay.setCircularBar(arrayCircularbar, 1);
    LabelConfig configUmidade = {
        .text = "", .prefix = "", .suffix = "%", .fontFamily = FONTE_UMIDADE,
        .datum = TL_DATUM, .fontColor = COR_DADOS, .backgroundColor = COR_FUNDO
    };
    textoUmidade.setup(configUmidade);
    LabelConfig configQualidade = {
        .text = "", .prefix = "", .suffix = "", .fontFamily = &RobotoRegular12pt7b,
        .datum = TL_DATUM, .fontColor = COR_DADOS, .backgroundColor = COR_FUNDO
    };
    textoQualidade.setup(configQualidade);
    LabelConfig configVentilacao = {
        .text = "", .prefix = "", .suffix = "%", .fontFamily = &RobotoBold7pt7b,
        .datum = TR_DATUM, .fontColor = COR_VENTILACAO, .backgroundColor = COR_PAINEL
    };
    textoVentilacao.setup(configVentilacao);
    LabelConfig configEstado = {
        .text = "", .prefix = "", .suffix = "", .fontFamily = &RobotoRegular5pt7b,
        .datum = TL_DATUM, .fontColor = COR_LEGENDA, .backgroundColor = COR_FUNDO
    };
    textoEstado.setup(configEstado);
    myDisplay.setLabel(arrayLabel, 4);
}

// 11. CALLBACKS: enviam comandos para a fila; o loop os processa.
void enviarComando(TipoComando tipo, int valor) {
    if (filaComandos == nullptr) return;
    Comando comando = {tipo, valor};
    if (xQueueSend(filaComandos, &comando, 0) != pdPASS) {
        Serial.println("Fila cheia: repita o toque.");
    }
}
void slider1_cb() { enviarComando(TipoComando::VENTILACAO, slider1.getValue()); }
void spinbox1_cb() { enviarComando(TipoComando::AJUSTE, spinbox1.getValue()); }
void toggle1_cb() { enviarComando(TipoComando::AUTO, toggle1.getStatus() ? 1 : 0); }
void textbutton1_cb() { enviarComando(TipoComando::ECO, 0); }
void textbutton1copy_cb() { enviarComando(TipoComando::CONFORTO, 0); }
void textbutton1copy2_cb() { enviarComando(TipoComando::TURBO, 0); }

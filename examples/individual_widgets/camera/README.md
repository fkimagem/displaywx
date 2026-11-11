# CameraMipi e TextButton: vídeo e enquadramento

Um **widget** é uma peça da tela, como um botão, uma barra ou uma etiqueta. Uma janela mostra a câmera ao vivo. Um botão abre controles para escolher o centro da imagem e o zoom.

Este manual acompanha [camera.ino](camera.ino). A referência é o código local da **DisplayWX 1.0.33**, incluindo os `.h` e `.cpp` atuais dos widgets.

![Tela do exemplo: CameraMipi e TextButton: vídeo e enquadramento.](screenshot.png)

*Imagem que acompanha o exemplo. Ela ilustra a disposição das peças; valores, cores ativas e textos podem mudar durante a execução. Confira os estados iniciais descritos abaixo.*

## Caminho da leitura

1. [Entenda o que cada peça faz](#1-o-que-este-exemplo-faz).
2. [Prepare a placa, os fios e as bibliotecas](#2-prepare-a-placa-e-as-bibliotecas).
3. [Copie o programa completo](#3-programa-completo).
4. [Aprenda os objetos, as fichas, os callbacks e o loop](#4-o-programa-explicado-por-partes).
5. [Consulte os métodos disponíveis para seu programa](#5-métodos-dos-widgets-para-usar-no-seu-programa).
6. [Faça uma experiência](#6-experiência-opcional).
7. [Confira o resultado](#7-como-conferir-se-funcionou).

## 1. O que este exemplo faz

O setup inicia a câmera, prepara o processamento de imagem, liga o painel e o toque e registra os widgets. A tarefa da biblioteca recebe os quadros de vídeo. O loop só espera 100 ms. Config camera chama openConfig para abrir o ajuste de enquadramento.

| Peça no programa | Widget | Sua tarefa |
| --- | --- | --- |
| `textbutton1` | TextButton | Abre a configuração da câmera. |
| `cameramipi1` | CameraMipi | Miniatura de vídeo em (215,165), com 304 × 196 pixels. |

**TextButton:** Um botão com palavras escritas dentro. Ele executa uma ação, como somar, navegar ou conectar.

**CameraMipi:** Uma janela de vídeo ao vivo de uma câmera MIPI no ESP32-P4. Pode ampliar a imagem e abrir ajustes de enquadramento.

A imagem mostra a posição reservada ao vídeo, não uma foto capturada. A resolução pedida ao sensor é 1920 × 1080; a miniatura tem 304 × 196. São tamanhos diferentes com funções diferentes. O FPS configurado é 5, limitado pelo desempenho disponível. O controle de brilho do painel é feito também pelo auxiliar Waveshare.

Faça toques curtos, tirando o dedo antes do próximo. Alguns controles podem repetir a ação se você mantiver o dedo sobre eles. Arraste apenas os controles que foram feitos para deslizar.

## 2. Prepare a placa e as bibliotecas

Este exemplo usa **ESP32-P4**. O conjunto é diferente dos exemplos ST7796 com ESP32-S3. Use o pacote **esp32 by Espressif Systems 3.3.12**, a DisplayWX **1.0.33** deste projeto e **GFX Library for Arduino 1.6.5**. `ESP_Video.h` vem da biblioteca ESP_Video incluída no pacote ESP32 desta instalação. Se não for encontrado, confira a instalação e a placa P4 selecionada.

Este exemplo usa ESP32-P4 com memória adequada à captura, painel Waveshare DSI de 7 polegadas ILI9881C (720 × 1280), toque GT911 Waveshare e câmera MIPI OV5647. Conecte painel ao conector DSI e câmera ao conector CSI compatíveis da placa, com cabos e alimentação indicados pelos módulos. SDA 7 e SCL 8 são compartilhados pelo controle de câmera, toque e auxiliar do painel. Não são os fios que transportam o vídeo: esse tráfego usa os conectores MIPI. O exemplo também leva GPIO 23 a HIGH como DISP_LED; confira se esse sinal está disponível na sua placa.

No [user_setup.h da raiz da biblioteca](../../../user_setup.h), deixe ativas estas opções e desative as outras de tela, toque e câmera:

```cpp
#define DISP_DEFAULT
#define TOUCH_GT911_WAVESHARE
#define CAM_OV5647
```

Selecione na IDE o modelo ESP32-P4 correspondente à sua placa e a configuração de memória/PSRAM indicada pelo fabricante. O painel usa resolução 720 × 1280 e rotação 0. A OV5647 usa o conector CSI; o painel, DSI. O controle I2C usa SDA 7 e SCL 8. Um display SPI ST7796 e uma câmera USB não substituem esses módulos neste código.

### Como carregar

1. Copie a pasta inteira deste exemplo para uma pasta de trabalho. Mantenha o nome da pasta igual ao nome do `.ino`.
2. Abra o `.ino` na IDE Arduino e substitua seu conteúdo pelo **programa completo abaixo**. Mantenha os arquivos `.h` que acompanham o exemplo.
3. Configure o `user_setup.h`, a placa e a porta como indicado acima.
4. Confira os fios, clique em **Verificar** e depois em **Carregar**.
5. Abra o Monitor Serial em **115200 baud** e reinicie a placa para acompanhar o início.

## 3. Programa completo

Copie o bloco inteiro, da primeira linha à última chave. Ele inclui a preparação da tela e todas as funções usadas. Os trechos isolados da próxima seção explicam este mesmo programa; não devem ser acrescentados novamente.

```cpp
#include <SPI.h>
#include <Arduino_GFX_Library.h>
#include <displaywx.h>
#include <ESP_Video.h>

#define FORMAT_SPIFFS_IF_FAILED false
const int DISPLAY_W = 720;
const int DISPLAY_H = 1280;
const int TOUCH_MAP_X0 = 0;
const int TOUCH_MAP_X1 = 720;
const int TOUCH_MAP_Y0 = 0;
const int TOUCH_MAP_Y1 = 1280;
const bool TOUCH_SWAP_XY = false;
const bool TOUCH_INVERT_X = false;
const bool TOUCH_INVERT_Y = false;
const int CAM_SDA = 7;
const int CAM_SCL = 8;
const int CAM_FREQUENCY_SCCB = 400000;
const int CAM_INT = -1;
const int CAM_RST = -1;
const int CAM_WIDTH = 1920;
const int CAM_HEIGHT = 1080;
const int CAM_I2C_PORT = 0;
const bool CAM_MIRROR_X = false;
const bool CAM_MIRROR_Y = false;
const int DISP_RST = -1;
const int TOUCH_SCL = 8;
const int TOUCH_SDA = 7;
const int TOUCH_RST = -1;
const uint8_t rotationScreen = 0;
const bool isIPS = false;

void screen0();
void loadWidgets();

void textbutton1_cb();

#define DISP_LED 23
Arduino_ESP32DSIPanel *bus = nullptr;
Arduino_DSI_Display *tft = nullptr;
DisplayWX myDisplay;

// textbutton1: Abre a configuração da câmera.
TextButton textbutton1(285, 430, 0);
const uint8_t qtdTextButton = 1;
TextButton *arrayTextButton[qtdTextButton] = {&textbutton1};
// cameramipi1: Miniatura de vídeo em (215,165), com 304 × 196 pixels.
CameraMipi cameramipi1(215, 165, 0);
const uint8_t qtdCameraMipi = 1;
CameraMipi *arrayCameraMipi[qtdCameraMipi] = {&cameramipi1};

void setup(){

    Serial.begin(115200);

    myDisplay.setRotationCamera(rotationScreen, CAM_MIRROR_X, CAM_MIRROR_Y);
    if(myDisplay.startCamera(CAM_I2C_PORT, CAM_SCL, CAM_SDA, CAM_FREQUENCY_SCCB, CAM_RST, CAM_INT, CAM_WIDTH, CAM_HEIGHT)){
        myDisplay.getCamera().startPpa();
    }else{
        Serial.println("Camera not started");
    }
    if (!myDisplay.waveshareDsiAuxBegin(ILI9881C_WAVESHARE_7_SDA, ILI9881C_WAVESHARE_7_SCL)) {
    Serial.println("AVISO: controlador auxiliar do painel falhou");
    }
    bus = new Arduino_ESP32DSIPanel(
    ILI9881C_WAVESHARE_7_HSYNC_PULSE, ILI9881C_WAVESHARE_7_HSYNC_BACK, ILI9881C_WAVESHARE_7_HSYNC_FRONT,
    ILI9881C_WAVESHARE_7_VSYNC_PULSE, ILI9881C_WAVESHARE_7_VSYNC_BACK, ILI9881C_WAVESHARE_7_VSYNC_FRONT,
    ILI9881C_WAVESHARE_7_PIXEL_CLOCK_HZ, ILI9881C_WAVESHARE_7_LANE_MBPS);
    tft = new Arduino_DSI_Display(
    720 , 1280 , bus, rotationScreen, true,
    DISP_RST, ili9881c_waveshare_7_init_operations, sizeof(ili9881c_waveshare_7_init_operations) / sizeof(lcd_init_cmd_t));
    tft->begin();
    myDisplay.waveshareDsiAuxBacklight(255);
    #if defined(DISP_LED)
    pinMode(DISP_LED, OUTPUT);
    digitalWrite(DISP_LED, HIGH);
    #endif
    myDisplay.setDrawObject(tft);

    myDisplay.setTouchCorners(TOUCH_MAP_X0, TOUCH_MAP_X1, TOUCH_MAP_Y0, TOUCH_MAP_Y1);
    myDisplay.setInvertAxis(TOUCH_INVERT_X, TOUCH_INVERT_Y);
    myDisplay.setSwapAxis(TOUCH_SWAP_XY);
    // Inicia o controlador de toque escolhido em user_setup.h.
    myDisplay.startTouchGT911Waveshare(DISPLAY_W, DISPLAY_H, rotationScreen, TOUCH_SDA, TOUCH_SCL);

    loadWidgets();
    myDisplay.loadScreen(screen0);
    // Inicia o trabalho da biblioteca que atende o toque e atualiza a tela.
    myDisplay.createTask(false, 3);
}

void loop(){

    delay(100);
}

void screen0(){

    myDisplay.setBackgroundColor(CFK_WHITE);

    myDisplay.drawWidgetsOnScreen(0);
}

void loadWidgets(){

    TextButtonConfig configTextButton0 = {
            .text = "Config camera",
            // Função chamada quando o usuário aciona este controle.
            .callback = textbutton1_cb,
            .fontFamily = &RobotoBold10pt7b,
            .width = 173,
            .height = 66,
            .radius = 10,
            .backgroundColor = CFK_COLOR062,
            .textColor = CFK_WHITE
        };

    // Aplica a ficha ao objeto; faça isso durante a preparação.
    textbutton1.setup(configTextButton0);
    myDisplay.setTextButton(arrayTextButton,qtdTextButton);

    CameraMipiConfig configCameraMipi0 = {
            .width = 304,
            .height = 196,
            .showZoomLevel = false,
            .showFps = false,
            .fps = 5
        };

    // Aplica a ficha ao objeto; faça isso durante a preparação.
    cameramipi1.setup(configCameraMipi0);
    myDisplay.setCameraMipi(arrayCameraMipi,qtdCameraMipi);

}

void textbutton1_cb(){
    cameramipi1.openConfig();
    Serial.print("Clicked on: ");Serial.println("textbutton1_cb");
}
```

## 4. O programa explicado por partes

### 4.1 Objetos, variáveis e listas

Um **objeto** é uma peça que seu programa pode chamar pelo nome. Uma **variável** é uma caixinha que guarda um valor. `bool` guarda `true` ou `false`; `int` guarda um inteiro; `float` e `double` guardam números com parte decimal. `const` indica algo que não será trocado durante a execução.


```cpp
// textbutton1: Abre a configuração da câmera.
TextButton textbutton1(285, 430, 0);
// Quantidade de objetos desta lista.
const uint8_t qtdTextButton = 1;
// Lista dos endereços dos widgets que a DisplayWX vai cuidar.
TextButton *arrayTextButton[qtdTextButton] = {&textbutton1};
// cameramipi1: Miniatura de vídeo em (215,165), com 304 × 196 pixels.
CameraMipi cameramipi1(215, 165, 0);
// Quantidade de objetos desta lista.
const uint8_t qtdCameraMipi = 1;
// Lista dos endereços dos widgets que a DisplayWX vai cuidar.
CameraMipi *arrayCameraMipi[qtdCameraMipi] = {&cameramipi1};
```

| Objeto | Como interpretar a criação |
| --- | --- |
| `textbutton1` (TextButton) | X = 285, Y = 430, tela = 0. Referência da posição: canto superior esquerdo. |
| `cameramipi1` (CameraMipi) | X = 215, Y = 165, tela = 0. Referência da posição: canto superior esquerdo. |

X cresce para a direita e Y cresce para baixo, a partir do canto superior esquerdo da tela. O número da tela decide em qual página a peça aparece. A primeira tela é 0.

O `&` antes do nome entrega o **endereço** do objeto, como informar onde fica uma casa. O `*` na declaração da lista indica que ela guarda endereços. `qtd...` precisa corresponder à quantidade de itens: ao acrescentar um widget, acrescente seu endereço e ajuste a quantidade. Mantenha os objetos e essas listas globais, fora das funções, para continuarem existindo enquanto a interface trabalha.

### 4.2 A ficha de configuração (`struct`)

A `struct` funciona como uma ficha: cada campo escolhe uma característica. O ponto em `.width` significa “preencha o campo largura”. Depois, `objeto.setup(ficha)` entrega a ficha ao widget. Configure cada peça uma vez em `loadWidgets()`, depois de preparar o display e antes de iniciar a tarefa.

#### `configTextButton0`: ficha de `textbutton1`

```cpp
TextButtonConfig configTextButton0 = {
            .text = "Config camera",
            // Função chamada quando o usuário aciona este controle.
            .callback = textbutton1_cb,
            .fontFamily = &RobotoBold10pt7b,
            .width = 173,
            .height = 66,
            .radius = 10,
            .backgroundColor = CFK_COLOR062,
            .textColor = CFK_WHITE
        };
// Aplica a ficha ao objeto; faça isso durante a preparação.
textbutton1.setup(configTextButton0);
```

| Campo | Valor nesta ficha | Para que serve |
| --- | --- | --- |
| `text` | `"Config camera"` | Texto inicial. Escreva entre aspas, por exemplo `"Olá"`. |
| `callback` | `textbutton1_cb` | Nome da função chamada quando há uma ação. Escreva o nome sem `()`. `nullptr` significa sem callback. |
| `fontFamily` | `&RobotoBold10pt7b` | Fonte das letras, por exemplo `&RobotoRegular10pt7b`. O `&` aponta para uma fonte que já existe. |
| `width` | `173` | Largura em pixels, os pontinhos da imagem. Mantenha o desenho dentro da tela. |
| `height` | `66` | Altura em pixels. Reserve espaço suficiente para o desenho e as letras. |
| `radius` | `10` | Raio do arredondamento dos cantos, não o tamanho do botão. |
| `backgroundColor` | `CFK_COLOR062` | Cor do fundo, por exemplo `CFK_WHITE`. Use a cor da área onde o widget aparece. |
| `textColor` | `CFK_WHITE` | Cor das letras/números. |

Abre a configuração da câmera.

#### `configCameraMipi0`: ficha de `cameramipi1`

```cpp
CameraMipiConfig configCameraMipi0 = {
            .width = 304,
            .height = 196,
            .showZoomLevel = false,
            .showFps = false,
            .fps = 5
        };
// Aplica a ficha ao objeto; faça isso durante a preparação.
cameramipi1.setup(configCameraMipi0);
```

| Campo | Valor nesta ficha | Para que serve |
| --- | --- | --- |
| `width` | `304` | Largura em pixels, os pontinhos da imagem. Mantenha o desenho dentro da tela. |
| `height` | `196` | Altura em pixels. Reserve espaço suficiente para o desenho e as letras. |
| `showZoomLevel` | `false` | `true` mostra o nível de zoom na miniatura; `false` esconde a indicação. |
| `showFps` | `false` | `true` mostra a taxa de atualização; `false` esconde a indicação. |
| `fps` | `5` | Taxa desejada da miniatura em quadros por segundo. O widget limita o valor à faixa de 5 a 24; não é garantia da taxa atingida. |

Miniatura de vídeo em (215,165), com 304 × 196 pixels.

Após configurar, registre cada lista. É assim que a DisplayWX sabe quais peças precisa acompanhar:

```cpp
// Registra a lista e sua quantidade para a biblioteca encontrar estes widgets.
myDisplay.setTextButton(arrayTextButton,qtdTextButton);
// Registra a lista e sua quantidade para a biblioteca encontrar estes widgets.
myDisplay.setCameraMipi(arrayCameraMipi,qtdCameraMipi);
```

`nullptr` significa “não liguei nada aqui”. Por exemplo, sem callback não há uma ação do seu programa para executar; sem `subtitle`, não há Label associado. Uma cor `CFK_...` é um valor de cor pronto da biblioteca. Uma fonte como `&RobotoRegular10pt7b` escolhe o desenho das letras. Para começar, conserve as fontes, os tamanhos e a ordem dos campos do exemplo.

### 4.3 O início: `setup()`

O Arduino executa `setup()` uma vez ao ligar. Ele prepara a comunicação, o display, o toque, os widgets e os componentes extras. A tarefa criada no fim continua cuidando da interface enquanto o seu `loop()` trabalha.

```cpp
void setup(){

    // Abre as mensagens de diagnóstico no computador.
    Serial.begin(115200);

    myDisplay.setRotationCamera(rotationScreen, CAM_MIRROR_X, CAM_MIRROR_Y);
    if(myDisplay.startCamera(CAM_I2C_PORT, CAM_SCL, CAM_SDA, CAM_FREQUENCY_SCCB, CAM_RST, CAM_INT, CAM_WIDTH, CAM_HEIGHT)){
        myDisplay.getCamera().startPpa();
    }else{
        Serial.println("Camera not started");
    }
    if (!myDisplay.waveshareDsiAuxBegin(ILI9881C_WAVESHARE_7_SDA, ILI9881C_WAVESHARE_7_SCL)) {
    Serial.println("AVISO: controlador auxiliar do painel falhou");
    }
    bus = new Arduino_ESP32DSIPanel(
    ILI9881C_WAVESHARE_7_HSYNC_PULSE, ILI9881C_WAVESHARE_7_HSYNC_BACK, ILI9881C_WAVESHARE_7_HSYNC_FRONT,
    ILI9881C_WAVESHARE_7_VSYNC_PULSE, ILI9881C_WAVESHARE_7_VSYNC_BACK, ILI9881C_WAVESHARE_7_VSYNC_FRONT,
    ILI9881C_WAVESHARE_7_PIXEL_CLOCK_HZ, ILI9881C_WAVESHARE_7_LANE_MBPS);
    tft = new Arduino_DSI_Display(
    720 , 1280 , bus, rotationScreen, true,
    DISP_RST, ili9881c_waveshare_7_init_operations, sizeof(ili9881c_waveshare_7_init_operations) / sizeof(lcd_init_cmd_t));
    tft->begin();
    myDisplay.waveshareDsiAuxBacklight(255);
    #if defined(DISP_LED)
    // OUTPUT comanda uma saída; INPUT lê um sinal externo.
    pinMode(DISP_LED, OUTPUT);
    // Comanda uma saída física: HIGH liga o sinal; LOW o desliga.
    digitalWrite(DISP_LED, HIGH);
    #endif
    // Entrega à DisplayWX o objeto que desenha no display.
    myDisplay.setDrawObject(tft);

    // Informa a faixa de coordenadas recebida do toque.
    myDisplay.setTouchCorners(TOUCH_MAP_X0, TOUCH_MAP_X1, TOUCH_MAP_Y0, TOUCH_MAP_Y1);
    myDisplay.setInvertAxis(TOUCH_INVERT_X, TOUCH_INVERT_Y);
    myDisplay.setSwapAxis(TOUCH_SWAP_XY);
    // Inicia o controlador de toque escolhido em user_setup.h.
    myDisplay.startTouchGT911Waveshare(DISPLAY_W, DISPLAY_H, rotationScreen, TOUCH_SDA, TOUCH_SCL);

    // Preenche as fichas, configura e registra os widgets.
    loadWidgets();
    // Pede a troca de tela; passe o nome da função, sem ().
    myDisplay.loadScreen(screen0);
    // Inicia o trabalho da biblioteca que atende o toque e atualiza a tela.
    myDisplay.createTask(false, 3);
}
```

As declarações como `void screen0();` no começo do programa são **avisos de que essas funções existem**. Elas permitem usar seus nomes nas fichas antes de chegar à parte que contém o corpo da função. Não apague esses avisos ao copiar trechos.

### 4.4 Callbacks: o que acontece após a ação

Um **callback** é a tarefa que você escolhe para acontecer quando um controle é acionado ou uma edição é confirmada. Na ficha, escreva o nome **sem `()`**. A biblioteca o chama no momento adequado. Evite esperas longas dentro dele: guarde um pedido em uma variável e faça a parte demorada no `loop()`, quando necessário.

#### `textbutton1_cb()`

Abre o painel de enquadramento. openConfig retorna um bool; o sketch não usa o retorno, mas você pode conferir false para saber que a abertura não foi possível.

```cpp
void textbutton1_cb(){
    cameramipi1.openConfig();
    Serial.print("Clicked on: ");Serial.println("textbutton1_cb");
}
```

As mensagens da `Serial` servem para observar o programa; os widgets não precisam delas para desenhar.

### 4.5 O trabalho repetido: `loop()`

O setup inicia a câmera, prepara o processamento de imagem, liga o painel e o toque e registra os widgets. A tarefa da biblioteca recebe os quadros de vídeo. O loop só espera 100 ms. Config camera chama openConfig para abrir o ajuste de enquadramento.

```cpp
void loop(){

    // Pausa em milissegundos: 1000 ms equivalem a um segundo.
    delay(100);
}
```

### 4.6 As telas

Cada função `screen...` pinta o fundo, coloca os textos fixos e manda desenhar os widgets daquela página. Troque de página com `myDisplay.loadScreen(screen0)` ou com a função da outra tela; passe o nome sem parênteses.

```cpp
void screen0(){

    // Pinta o fundo da tela com a cor escolhida.
    myDisplay.setBackgroundColor(CFK_WHITE);

    // Desenha os widgets registrados para esse número de tela.
    myDisplay.drawWidgetsOnScreen(0);
}
```

### 4.7 Funções extras

Este exemplo não precisa de outras funções próprias além das que acabamos de estudar. As funções como `digitalWrite`, `delay` e `analogRead` pertencem ao ambiente Arduino.

## 5. Métodos dos widgets para usar no seu programa

Um **método** é uma ação que pertence a um objeto. Em `objeto.metodo(valor)`, primeiro vem o nome da peça, depois o ponto, a ação e suas informações entre parênteses. As tabelas usam objetos reais deste exemplo. Faça essas chamadas **depois da configuração**; a linha `setup(...)` pertence à preparação. Os nomes das fichas só existem dentro de `loadWidgets()`.

### TextButton: métodos para seu programa

Um botão com palavras escritas dentro. Ele executa uma ação, como somar, navegar ou conectar.

| Método | Como usar | Resultado e cuidado |
| --- | --- | --- |
| `setup(config)` | `textbutton1.setup(configTextButton0);` | Aplica a ficha de configuração após preparar o display. Não retorna valor. Configure no `loadWidgets()` antes de iniciar a tarefa. |
| `onClick()` | `textbutton1.onClick();` | Solicita o callback pelo código. Não recebe argumento `triggerCallback` e não guarda estado ligado/desligado. |
| `getEnabled()` | `bool podeTocar = textbutton1.getEnabled();` | Retorna se o botão está habilitado para toque. |
| `setEnabled(valor)` | `textbutton1.setEnabled(false);` | `false` bloqueia o toque e `true` permite. Não muda automaticamente a cor; `onClick()` pelo código continua possível. |
| `show()` | `textbutton1.show();` | Torna visível na sua tela e pede atualização. Para TouchArea, volta a permitir toque; ela continua invisível. |
| `hide()` | `textbutton1.hide();` | Impede futuras atualizações visuais e, quando há toque, sua interação. Não apaga os pixels já desenhados: repinte a tela como explicado abaixo. |

O `radius` da configuração arredonda os cantos; não é o tamanho do botão. Use dimensões maiores que as letras. Para trocar o texto inicial, altere `.text` e carregue o programa novamente; não existe `setText()` nesta classe. Toques prolongados podem repetir ações.

### CameraMipi: métodos para seu programa

Uma janela de vídeo ao vivo de uma câmera MIPI no ESP32-P4. Pode ampliar a imagem e abrir ajustes de enquadramento.

| Método | Como usar | Resultado e cuidado |
| --- | --- | --- |
| `setup(config)` | `cameramipi1.setup(configCameraMipi0);` | Aplica a ficha de configuração após preparar o display. Não retorna valor. Configure no `loadWidgets()` antes de iniciar a tarefa. |
| `onClick(triggerCallback = true)` | `cameramipi1.onClick(false);` | Alterna entre miniatura e tela inteira. O argumento controla callback somente ao entrar; ao sair, solicita restauração da tela. Esta configuração de câmera não possui campo de callback. |
| `openConfig()` | `bool abriu = cameramipi1.openConfig();` | Abre os ajustes de centro e zoom. Retorna `true` ao abrir e `false` se câmera/tarefa/quadro não estiver disponível ou outra edição estiver aberta. |
| `isFullScreen()` | `bool ampliada = cameramipi1.isFullScreen();` | Informa se esta janela está em tela inteira. |
| `hasFullScreen()` | `bool existe = CameraMipi::hasFullScreen();` | Consulta se alguma janela de câmera está em tela inteira. |
| `leaveFullScreen()` | `cameramipi1.leaveFullScreen();` | Limpa apenas o estado de tela inteira desta janela. Para uma saída completa, prefira `onClick(false)` quando `isFullScreen()` for true. |
| `exitFullScreen()` | `CameraMipi::exitFullScreen();` | Limpa apenas a indicação compartilhada de tela inteira. Isoladamente não restaura a tela; prefira a operação completa acima. |
| `setZoomLevel(nivel)` | `cameramipi1.setZoomLevel(2);` | Define zoom inteiro, começando em 1. Zero é ignorado. O processamento pode limitar o zoom efetivo para caber na imagem. |
| `getZoomLevel()` | `uint8_t zoom = cameramipi1.getZoomLevel();` | Retorna o zoom solicitado. |
| `zoomLabel()` | `const char* texto = cameramipi1.zoomLabel();` | Retorna texto como `2x`, em memória interna do widget. |
| `setCenterPoint(x, y)` | `cameramipi1.setCenterPoint(-1, -1);` | Escolhe o centro do recorte nas coordenadas do quadro da câmera. Valores negativos pedem o centro automático; não são coordenadas do display. |
| `centerPointX()` | `int16_t x = cameramipi1.centerPointX();` | Lê a coordenada X do centro escolhido. |
| `centerPointY()` | `int16_t y = cameramipi1.centerPointY();` | Lê a coordenada Y do centro escolhido. |
| `setKeepZoomOnMaximized(valor)` | `cameramipi1.setKeepZoomOnMaximized(true);` | Escolhe se mantém o zoom ao ampliar para tela inteira. |
| `keepZoomOnMaximized()` | `bool manter = cameramipi1.keepZoomOnMaximized();` | Consulta essa escolha. |
| `getXPos()` | `int16_t x = cameramipi1.getXPos();` | Retorna a posição X configurada da miniatura. |
| `getYPos()` | `int16_t y = cameramipi1.getYPos();` | Retorna a posição Y configurada da miniatura. |
| `getWidth()` | `uint16_t largura = cameramipi1.getWidth();` | Retorna a largura configurada da miniatura. |
| `getHeight()` | `uint16_t altura = cameramipi1.getHeight();` | Retorna a altura configurada da miniatura. |
| `showZoomLevel()` | `bool mostra = cameramipi1.showZoomLevel();` | Consulta a opção de exibir o zoom; não é um comando para ativá-la. |
| `showFps()` | `bool mostra = cameramipi1.showFps();` | Consulta a opção de exibir FPS; não a altera. |
| `fps()` | `uint8_t taxa = cameramipi1.fps();` | Retorna a taxa configurada para a miniatura. |
| `getEnabled()` | `bool toque = cameramipi1.getEnabled();` | Consulta se aceita toque para ampliar. |
| `setEnabled(valor)` | `cameramipi1.setEnabled(false);` | Desabilita o toque com false; não interrompe a captura nem bloqueia chamadas pelo código. |
| `setStatus(valor)` | `cameramipi1.setStatus(true);` | Apenas guarda um booleano. Não inicia/para a câmera nem muda tela inteira. Não possui argumento triggerCallback. |
| `getStatus()` | `bool valor = cameramipi1.getStatus();` | Lê o booleano de setStatus. Não indica se a câmera iniciou ou se está em tela inteira. |
| `changeState()` | `cameramipi1.changeState();` | Nesta implementação não altera o estado; não use para ampliar ou parar vídeo. |
| `lock()` | `cameramipi1.lock();` | Bloqueia a interação por toque enquanto você não chamar unlock(). Não é uma trava para organizar tarefas do processador. |
| `unlock()` | `cameramipi1.unlock();` | Remove esse bloqueio de toque; não desfaz setEnabled(false). |
| `isLocked()` | `bool bloqueado = cameramipi1.isLocked();` | Consulta se lock() está ativo. |
| `show()` | `cameramipi1.show();` | Existe, mas atualmente não muda a visibilidade. |
| `hide()` | `cameramipi1.hide();` | Existe, mas atualmente não esconde o vídeo. |

Na implementação atual, `show()` e `hide()` apenas registram mensagens de diagnóstico: não mostram/escondem a câmera. Para sair do vídeo ampliado, use `if (cameramipi1.isFullScreen()) cameramipi1.onClick(false);`. Para uma página sem vídeo, navegue para outra tela que não contenha esse widget.

### Esconder e mostrar na tela

`hide()` não funciona como uma borracha para os pixels que já foram pintados. Para remover o desenho da página atual, esconda a peça e peça que a página seja montada novamente. Para fazê-la voltar, use `show()`. O estado/valor continua guardado.

```cpp
// Execute uma vez, na tela 0 deste exemplo.
textbutton1.hide();
myDisplay.loadScreen(screen0);

// Em outra ação, quando quiser mostrar de novo:
textbutton1.show();
myDisplay.loadScreen(screen0);
```

Os dois blocos são ações separadas: se executar todos de uma vez, o widget voltará logo em seguida. Textos fixos feitos com `printText()` continuam aparecendo, pois pertencem à função de tela. Um widget da tela 1 só aparece quando você volta à tela 1.

O desenho periódico e a detecção de toque ficam a cargo da DisplayWX. Seu `.ino` configura, registra, consulta e altera as peças pelas chamadas acima; não precisa chamar rotinas internas de desenho ou de detecção.

## 6. Experiência opcional

Com a câmera ativa, pede zoom 2x e mantém a escolha ao ampliar. Para ajustar o centro de forma visual, prefira o botão Config camera.

```cpp
cameramipi1.setZoomLevel(2);
cameramipi1.setKeepZoomOnMaximized(true);
```

Para executar esse trecho uma vez sem repeti-lo em cada volta, coloque-o no lugar do comentário abaixo **dentro do loop existente**. Digite `t` no Monitor Serial e envie. Preserve as outras tarefas do loop; o trecho só roda quando chega essa letra.

```cpp
if (Serial.available() > 0) {
    char comando = Serial.read();
    if (comando == 't') {
        // Cole aqui o trecho da experiência.
    }
}
```

Mude uma coisa de cada vez. O programa completo da seção 3 continua sendo o ponto de partida para voltar ao comportamento original do exercício.

## 7. Como conferir se funcionou

1. Confira a montagem e observe a Serial: Camera not started indica falha na captura, não um problema do botão.
2. Aguarde a janela mostrar quadros da câmera.
3. Toque Config camera. Use zoom e setas para enquadrar; OK aplica, Cancel descarta.
4. Toque na miniatura para ampliar. Tire o dedo e toque novamente no vídeo ampliado para voltar à tela com o botão.

| Se acontecer... | Confira... |
| --- | --- |
| Erro ao compilar sobre tela, toque ou tipo desconhecido | Placa selecionada, versões instaladas e opções ativas no `user_setup.h` da raiz da biblioteca. Copie o programa inteiro. |
| Tela apagada ou sem desenho | Alimentação, GND, iluminação do módulo e fios/conectores do display. |
| Desenho correto, mas o toque não responde ou está deslocado | Controlador selecionado, SDA/SCL e configuração de rotação, limites e inversão dos eixos. Use os valores completos do exemplo para essa montagem. |
| Método com argumentos diferentes dá erro | Consulte a tabela da classe exata. Nem todos os `onClick` ou `setValue` recebem os mesmos argumentos. |

Este manual foi conferido com os arquivos locais da biblioteca. A validação descrita aqui é de código e documentação; a compilação para a sua placa e o ensaio físico devem ser feitos com a montagem indicada.

### Arquivos usados como referência

- [camera.ino](camera.ino).
- TextButton: [declarações `.h`](../../../src/widgets/textbutton/wtextbutton.h) e [implementação `.cpp`](../../../src/widgets/textbutton/wtextbutton.cpp).
- CameraMipi: [declarações `.h`](../../../src/widgets/cameramipi/wcameramipi.h) e [implementação `.cpp`](../../../src/widgets/cameramipi/wcameramipi.cpp).
- [DisplayWX](../../../src/displaywx.h) e [implementação da integração](../../../src/displaywx.cpp).
- [WidgetBase](../../../src/widgets/widgetbase.h) e [implementação](../../../src/widgets/widgetbase.cpp), para habilitação e bloqueio herdados.

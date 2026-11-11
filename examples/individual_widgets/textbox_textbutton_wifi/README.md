# TextBox e TextButton: preenchendo uma conexão Wi-Fi

Um **widget** é uma peça da tela, como um botão, uma barra ou uma etiqueta. Você digita o nome da rede e a senha usando a tela, toca Connect e acompanha a mensagem de conexão.

Este manual acompanha [textbox_textbutton_wifi.ino](textbox_textbutton_wifi.ino). A referência é o código local da **DisplayWX 1.0.33**, incluindo os `.h` e `.cpp` atuais dos widgets.

![Tela do exemplo: TextBox e TextButton: preenchendo uma conexão Wi-Fi.](screenshot.png)

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

Confirmar cada caixa com OK faz seu callback guardar o texto. Connect deixa `connectWiFi = true`, um bilhete para o loop iniciar a tentativa. O loop espera até conectar ou completar dez segundos e atualiza a mensagem de estado.

| Peça no programa | Widget | Sua tarefa |
| --- | --- | --- |
| `text3` | Label | Mostra Status: Connected ou Disconnected. |
| `tbssid` | TextBox | Nome da rede (SSID). |
| `tbpass` | TextBox | Senha da rede. |
| `textbutton1` | TextButton | Pede uma tentativa de conexão. |

**Label:** Uma etiqueta de texto. Mostra palavras ou números com prefixo e sufixo, mas não recebe toques.

**TextBox:** Uma caixa para escrever palavras usando o teclado da própria tela.

**TextButton:** Um botão com palavras escritas dentro. Ele executa uma ação, como somar, navegar ou conectar.

A senha aparece como texto comum no campo e no teclado: este widget não possui máscara de senha. Os textos SSDI e PASS são exemplos; as variáveis usadas na conexão começam vazias e são preenchidas pelos callbacks de confirmação. O botão não tenta conectar enquanto você ainda está editando o teclado.

Faça toques curtos, tirando o dedo antes do próximo. Alguns controles podem repetir a ação se você mantiver o dedo sobre eles. Arraste apenas os controles que foram feitos para deslizar.

## 2. Prepare a placa e as bibliotecas

Use uma placa **ESP32-S3**, display SPI **ST7796 de 320 × 480**, toque capacitivo **FT6336U**, cabo USB de dados e fios. A rotação 1 deixa uma área de desenho de **480 × 320 pixels**. Pixel é cada pontinho da imagem.

Na IDE Arduino, use a DisplayWX **1.0.33** deste projeto, **GFX Library for Arduino 1.6.5** e o pacote **esp32 by Espressif Systems 3.3.12**. O código da DisplayWX aceita as versões 3.3.11 e 3.3.12 desse pacote. Selecione o modelo ESP32-S3 da sua placa, sua porta USB e as opções de memória correspondentes ao hardware.

Abra o [user_setup.h da raiz da biblioteca](../../../user_setup.h), ao lado da pasta `src`. Deixe estas duas opções ativas:

```cpp
#define DISP_DEFAULT  // Display colorido.
#define TOUCH_FT6336  // Controlador FT6336U.
```

Desative as demais opções de display, toque e câmera colocando `//` no começo dessas linhas. Por exemplo: `// #define TOUCH_FT6236U`. O nome da opção correta é `TOUCH_FT6336`, sem U. Definir isso apenas no `.ino` não configura os outros arquivos da biblioteca.

### Ligações do display e do toque

| Sinal do módulo | GPIO do ESP32-S3 |
| --- | --- |
| MOSI / SDI do display | 11 |
| SCK / CLK | 12 |
| CS | 10 |
| DC / RS | 8 |
| RST do display | 9 |
| SDA do toque | 14 |
| SCL do toque | 13 |
| INT do toque | 18 |
| RST do toque | 17 |

`DISP_MISO = -1` significa que não há fio de leitura SPI. Una os GNDs. Alimente display e iluminação conforme o seu módulo; o sketch não controla um pino de backlight. Use sinais lógicos de 3,3 V. Faça a montagem com a alimentação desligada e ajuda de um adulto. Os números acima são GPIOs, não a posição física dos pinos no conector.

### Componentes deste exercício

Use uma rede Wi-Fi de 2,4 GHz. Toque nas duas caixas, limpe os textos de exemplo, digite nome e senha e confirme cada uma com OK. Os dados são usados na RAM durante a execução; este sketch não oferece uma rotina própria de salvar credenciais para restaurá-las ao iniciar.

### Como carregar

1. Copie a pasta inteira deste exemplo para uma pasta de trabalho. Mantenha o nome da pasta igual ao nome do `.ino`.
2. Abra o `.ino` na IDE Arduino e substitua seu conteúdo pelo **programa completo abaixo**. Mantenha os arquivos `.h` que acompanham o exemplo.
3. Configure o `user_setup.h`, a placa e a porta como indicado acima.
4. Confira os fios, clique em **Verificar** e depois em **Carregar**.
5. Abra o Monitor Serial em **115200 baud** e reinicie a placa para acompanhar o início. Se estiver usando a USB nativa do ESP32-S3, a opção **USB CDC On Boot**, quando disponível para sua placa, precisa estar habilitada para os diagnósticos nessa porta.

## 3. Programa completo

Copie o bloco inteiro, da primeira linha à última chave. Ele inclui a preparação da tela e todas as funções usadas. Os trechos isolados da próxima seção explicam este mesmo programa; não devem ser acrescentados novamente.

```cpp
#if CONFIG_IDF_TARGET_ESP32S2 || CONFIG_IDF_TARGET_ESP32S3
#define VSPI FSPI
#endif

#include <SPI.h>
#include <WiFi.h>
#include <Arduino_GFX_Library.h>

#include <displaywx.h>

#define FORMAT_SPIFFS_IF_FAILED false
const int DISPLAY_W = 480;
const int DISPLAY_H = 320;
const int DISP_FREQUENCY = 27000000;
const int TOUCH_MAP_X0 = 0;
const int TOUCH_MAP_Y0 = 0;
const int TOUCH_MAP_X1 = 320;
const int TOUCH_MAP_Y1 = 480;
const bool TOUCH_SWAP_XY = true;
const bool TOUCH_INVERT_X = false;
const bool TOUCH_INVERT_Y = true;
const int DISP_MOSI = 11;
const int DISP_MISO = -1;
const int DISP_SCLK = 12;
const int DISP_CS = 10;
const int DISP_DC = 8;
const int DISP_RST = 9;
const int TOUCH_SCL = 13;
const int TOUCH_SDA = 14;
const int TOUCH_INT = 18;
const int TOUCH_RST = 17;
const uint8_t rotationScreen = 1;
const bool isIPS = true;

void screen0();
void loadWidgets();

void tbssid_cb();
void tbpass_cb();
void textbutton1_cb();

#if defined(CONFIG_IDF_TARGET_ESP32S3)
SPIClass spi_shared(FSPI);
#else
SPIClass spi_shared(HSPI);
#endif
Arduino_DataBus *bus = nullptr;
Arduino_GFX *tft = nullptr;
DisplayWX myDisplay;

// text3: Mostra Status: Connected ou Disconnected.
Label text3(190, 255, 0);
const uint8_t qtdLabel = 1;
Label *arrayLabel[qtdLabel] = {&text3};
// tbssid: Nome da rede (SSID).
TextBox tbssid(175, 90, 0);
// tbpass: Senha da rede.
TextBox tbpass(175, 140, 0);
const uint8_t qtdTextbox = 2;
TextBox *arrayTextbox[qtdTextbox] = {&tbssid, &tbpass};
const char* tb0_val = "";
const char* tb1_val = "";
// textbutton1: Pede uma tentativa de conexão.
TextButton textbutton1(175, 190, 0);
const uint8_t qtdTextButton = 1;
TextButton *arrayTextButton[qtdTextButton] = {&textbutton1};

bool connectWiFi = false;
uint32_t timeOutConnectWiFi = 10000;
bool isWiFiConnected = false;

void setup(){

    Serial.begin(115200);

    spi_shared.begin(DISP_SCLK, DISP_MISO, DISP_MOSI);
    bus = new Arduino_HWSPI(DISP_DC, DISP_CS, DISP_SCLK, DISP_MOSI, DISP_MISO, &spi_shared);
    tft = new Arduino_ST7796(bus, DISP_RST, rotationScreen, isIPS, 320, 480);
    tft->begin(DISP_FREQUENCY);
    myDisplay.setDrawObject(tft);

    myDisplay.setTouchCorners(TOUCH_MAP_X0, TOUCH_MAP_X1, TOUCH_MAP_Y0, TOUCH_MAP_Y1);
    myDisplay.setInvertAxis(TOUCH_INVERT_X, TOUCH_INVERT_Y);
    myDisplay.setSwapAxis(TOUCH_SWAP_XY);
    // Inicia o controlador de toque escolhido em user_setup.h.
    myDisplay.startTouchFT6336(DISPLAY_W, DISPLAY_H, rotationScreen, TOUCH_SDA, TOUCH_SCL, TOUCH_INT, TOUCH_RST);

    loadWidgets();
    myDisplay.loadScreen(screen0);
    // Inicia o trabalho da biblioteca que atende o toque e atualiza a tela.
    myDisplay.createTask(false, 3);
}

void loop(){

    if(connectWiFi){
        connectWiFi = false;
        WiFi.begin(tb0_val, tb1_val);
        uint32_t startTime = millis();
        while(WiFi.status() != WL_CONNECTED && millis() - startTime < timeOutConnectWiFi){
            delay(1000);
        }
    }

    if(WiFi.status() == WL_CONNECTED){
        Serial.println("WiFi connected");
        isWiFiConnected = true;
    }else{
        Serial.println("WiFi connection failed");
        isWiFiConnected = false;
    }

    myDisplay.startCustomDraw();
    text3.setText(isWiFiConnected ? "Connected" : "Disconnected");
    myDisplay.finishCustomDraw();
    delay(2000);
}

void screen0(){

    myDisplay.setBackgroundColor(CFK_WHITE);
    myDisplay.printText("SSID", 120, 92, TL_DATUM, CFK_BLACK, CFK_WHITE, &RobotoRegular10pt7b);

    myDisplay.printText("Pass", 120, 142, TL_DATUM, CFK_BLACK, CFK_WHITE, &RobotoRegular10pt7b);

    myDisplay.printText("Connect WiFi", 175, 22, TL_DATUM, CFK_BLACK, CFK_WHITE, &RobotoRegular10pt7b);
    myDisplay.drawWidgetsOnScreen(0);
}

void loadWidgets(){

    LabelConfig configLabel0 = {
            .text = "---",
            .prefix = "Status: ",
            .suffix = "",
            .fontFamily = &RobotoRegular10pt7b,
            .datum = TL_DATUM,
            .fontColor = CFK_BLACK,
            .backgroundColor = CFK_WHITE
        };
    // Aplica a ficha ao objeto; faça isso durante a preparação.
    text3.setup(configLabel0);
    myDisplay.setLabel(arrayLabel,qtdLabel);

    WKeyboard::m_backgroundColor = CFK_GREY3;
    WKeyboard::m_letterColor = CFK_BLACK;
    WKeyboard::m_keyColor = CFK_GREY13;

    TextBoxConfig configTextBox0 = {
            .startValue = "SSDI",
            // Tela para onde o teclado deve voltar ao fechar.
            .funcPtr = screen0,
            // Função chamada quando o usuário confirma/aciona este controle.
            .cb = tbssid_cb,
            .font = &RobotoRegular10pt7b,
            .width = 141,
            .height = 25,
            .letterColor = CFK_GREY1,
            .backgroundColor = CFK_WHITE
        };
    // Aplica a ficha ao objeto; faça isso durante a preparação.
    tbssid.setup(configTextBox0);
    TextBoxConfig configTextBox1 = {
            .startValue = "PASS",
            // Tela para onde o teclado deve voltar ao fechar.
            .funcPtr = screen0,
            // Função chamada quando o usuário confirma/aciona este controle.
            .cb = tbpass_cb,
            .font = &RobotoRegular10pt7b,
            .width = 141,
            .height = 25,
            .letterColor = CFK_GREY1,
            .backgroundColor = CFK_WHITE
        };
    // Aplica a ficha ao objeto; faça isso durante a preparação.
    tbpass.setup(configTextBox1);
    myDisplay.setTextbox(arrayTextbox,qtdTextbox);

    TextButtonConfig configTextButton0 = {
            .text = "Connect",
            // Função chamada quando o usuário aciona este controle.
            .callback = textbutton1_cb,
            .fontFamily = &RobotoBold10pt7b,
            .width = 110,
            .height = 40,
            .radius = 10,
            .backgroundColor = CFK_COLOR17,
            .textColor = CFK_BLACK
        };

    // Aplica a ficha ao objeto; faça isso durante a preparação.
    textbutton1.setup(configTextButton0);
    myDisplay.setTextButton(arrayTextButton,qtdTextButton);

}

void tbssid_cb(){

    tb0_val = tbssid.getValue();
}

void tbpass_cb(){

    tb1_val = tbpass.getValue();
}

void textbutton1_cb(){
    connectWiFi = true;
    Serial.print("Clicked on: ");Serial.println("textbutton1_cb");
}
```

## 4. O programa explicado por partes

### 4.1 Objetos, variáveis e listas

Um **objeto** é uma peça que seu programa pode chamar pelo nome. Uma **variável** é uma caixinha que guarda um valor. `bool` guarda `true` ou `false`; `int` guarda um inteiro; `float` e `double` guardam números com parte decimal. `const` indica algo que não será trocado durante a execução.


```cpp
// text3: Mostra Status: Connected ou Disconnected.
Label text3(190, 255, 0);
// Quantidade de objetos desta lista.
const uint8_t qtdLabel = 1;
// Lista dos endereços dos widgets que a DisplayWX vai cuidar.
Label *arrayLabel[qtdLabel] = {&text3};
// tbssid: Nome da rede (SSID).
TextBox tbssid(175, 90, 0);
// tbpass: Senha da rede.
TextBox tbpass(175, 140, 0);
// Quantidade de objetos desta lista.
const uint8_t qtdTextbox = 2;
// Lista dos endereços dos widgets que a DisplayWX vai cuidar.
TextBox *arrayTextbox[qtdTextbox] = {&tbssid, &tbpass};
const char* tb0_val = "";
const char* tb1_val = "";
// textbutton1: Pede uma tentativa de conexão.
TextButton textbutton1(175, 190, 0);
// Quantidade de objetos desta lista.
const uint8_t qtdTextButton = 1;
// Lista dos endereços dos widgets que a DisplayWX vai cuidar.
TextButton *arrayTextButton[qtdTextButton] = {&textbutton1};

bool connectWiFi = false;
uint32_t timeOutConnectWiFi = 10000;
bool isWiFiConnected = false;
```

| Objeto | Como interpretar a criação |
| --- | --- |
| `text3` (Label) | X = 190, Y = 255, tela = 0. Referência da posição: ponto de alinhamento escolhido por datum. |
| `tbssid` (TextBox) | X = 175, Y = 90, tela = 0. Referência da posição: canto superior esquerdo. |
| `tbpass` (TextBox) | X = 175, Y = 140, tela = 0. Referência da posição: canto superior esquerdo. |
| `textbutton1` (TextButton) | X = 175, Y = 190, tela = 0. Referência da posição: canto superior esquerdo. |

X cresce para a direita e Y cresce para baixo, a partir do canto superior esquerdo da tela. O número da tela decide em qual página a peça aparece. A primeira tela é 0.

O `&` antes do nome entrega o **endereço** do objeto, como informar onde fica uma casa. O `*` na declaração da lista indica que ela guarda endereços. `qtd...` precisa corresponder à quantidade de itens: ao acrescentar um widget, acrescente seu endereço e ajuste a quantidade. Mantenha os objetos e essas listas globais, fora das funções, para continuarem existindo enquanto a interface trabalha.

### 4.2 A ficha de configuração (`struct`)

A `struct` funciona como uma ficha: cada campo escolhe uma característica. O ponto em `.width` significa “preencha o campo largura”. Depois, `objeto.setup(ficha)` entrega a ficha ao widget. Configure cada peça uma vez em `loadWidgets()`, depois de preparar o display e antes de iniciar a tarefa.

#### `configLabel0`: ficha de `text3`

```cpp
LabelConfig configLabel0 = {
            .text = "---",
            .prefix = "Status: ",
            .suffix = "",
            .fontFamily = &RobotoRegular10pt7b,
            .datum = TL_DATUM,
            .fontColor = CFK_BLACK,
            .backgroundColor = CFK_WHITE
        };
// Aplica a ficha ao objeto; faça isso durante a preparação.
text3.setup(configLabel0);
```

| Campo | Valor nesta ficha | Para que serve |
| --- | --- | --- |
| `text` | `"---"` | Texto inicial. Escreva entre aspas, por exemplo `"Olá"`. |
| `prefix` | `"Status: "` | Texto colocado antes do valor, como `"Valor: "`. `""` deixa vazio. |
| `suffix` | `""` | Texto colocado depois do valor, como `" V"`. `""` deixa vazio. |
| `fontFamily` | `&RobotoRegular10pt7b` | Fonte das letras, por exemplo `&RobotoRegular10pt7b`. O `&` aponta para uma fonte que já existe. |
| `datum` | `TL_DATUM` | Alinhamento do texto em relação a `(x,y)`: `TL_DATUM` usa o canto superior esquerdo; `TC_DATUM`, o centro do topo. |
| `fontColor` | `CFK_BLACK` | Cor das letras, usando os nomes `CFK_...`. |
| `backgroundColor` | `CFK_WHITE` | Cor do fundo, por exemplo `CFK_WHITE`. Use a cor da área onde o widget aparece. |

Mostra Status: Connected ou Disconnected.

#### `configTextBox0`: ficha de `tbssid`

```cpp
TextBoxConfig configTextBox0 = {
            // Valor que aparece ao iniciar a caixa.
            .startValue = "SSDI",
            // Tela para onde o teclado deve voltar ao fechar.
            .funcPtr = screen0,
            // Função chamada quando o usuário confirma/aciona este controle.
            .cb = tbssid_cb,
            .font = &RobotoRegular10pt7b,
            .width = 141,
            .height = 25,
            .letterColor = CFK_GREY1,
            .backgroundColor = CFK_WHITE
        };
// Aplica a ficha ao objeto; faça isso durante a preparação.
tbssid.setup(configTextBox0);
```

| Campo | Valor nesta ficha | Para que serve |
| --- | --- | --- |
| `startValue` | `"SSDI"` | Texto inicial entre aspas. Neste exemplo é um lembrete para você digitar os dados. |
| `funcPtr` | `screen0` | Função que monta a tela de retorno, como `screen0`, sem `()`. O teclado usa essa tela ao fechar. |
| `cb` | `tbssid_cb` | Nome da função chamada após a ação. Escreva sem `()`, ou use `nullptr` para não associar uma ação. |
| `font` | `&RobotoRegular10pt7b` | Fonte das letras. O `&` antes do nome aponta para a fonte escolhida. |
| `width` | `141` | Largura em pixels, os pontinhos da imagem. Mantenha o desenho dentro da tela. |
| `height` | `25` | Altura em pixels. Reserve espaço suficiente para o desenho e as letras. |
| `letterColor` | `CFK_GREY1` | Cor das letras ou números da caixa. |
| `backgroundColor` | `CFK_WHITE` | Cor do fundo, por exemplo `CFK_WHITE`. Use a cor da área onde o widget aparece. |

Nome da rede (SSID).

#### `configTextBox1`: ficha de `tbpass`

```cpp
TextBoxConfig configTextBox1 = {
            // Valor que aparece ao iniciar a caixa.
            .startValue = "PASS",
            // Tela para onde o teclado deve voltar ao fechar.
            .funcPtr = screen0,
            // Função chamada quando o usuário confirma/aciona este controle.
            .cb = tbpass_cb,
            .font = &RobotoRegular10pt7b,
            .width = 141,
            .height = 25,
            .letterColor = CFK_GREY1,
            .backgroundColor = CFK_WHITE
        };
// Aplica a ficha ao objeto; faça isso durante a preparação.
tbpass.setup(configTextBox1);
```

Senha da rede.

#### `configTextButton0`: ficha de `textbutton1`

```cpp
TextButtonConfig configTextButton0 = {
            .text = "Connect",
            // Função chamada quando o usuário aciona este controle.
            .callback = textbutton1_cb,
            .fontFamily = &RobotoBold10pt7b,
            .width = 110,
            .height = 40,
            .radius = 10,
            .backgroundColor = CFK_COLOR17,
            .textColor = CFK_BLACK
        };
// Aplica a ficha ao objeto; faça isso durante a preparação.
textbutton1.setup(configTextButton0);
```

| Campo | Valor nesta ficha | Para que serve |
| --- | --- | --- |
| `text` | `"Connect"` | Texto inicial. Escreva entre aspas, por exemplo `"Olá"`. |
| `callback` | `textbutton1_cb` | Nome da função chamada quando há uma ação. Escreva o nome sem `()`. `nullptr` significa sem callback. |
| `fontFamily` | `&RobotoBold10pt7b` | Fonte das letras, por exemplo `&RobotoRegular10pt7b`. O `&` aponta para uma fonte que já existe. |
| `width` | `110` | Largura em pixels, os pontinhos da imagem. Mantenha o desenho dentro da tela. |
| `height` | `40` | Altura em pixels. Reserve espaço suficiente para o desenho e as letras. |
| `radius` | `10` | Raio do arredondamento dos cantos, não o tamanho do botão. |
| `backgroundColor` | `CFK_COLOR17` | Cor do fundo, por exemplo `CFK_WHITE`. Use a cor da área onde o widget aparece. |
| `textColor` | `CFK_BLACK` | Cor das letras/números. |

Pede uma tentativa de conexão.

Após configurar, registre cada lista. É assim que a DisplayWX sabe quais peças precisa acompanhar:

```cpp
// Registra a lista e sua quantidade para a biblioteca encontrar estes widgets.
myDisplay.setLabel(arrayLabel,qtdLabel);
// Registra a lista e sua quantidade para a biblioteca encontrar estes widgets.
myDisplay.setTextbox(arrayTextbox,qtdTextbox);
// Registra a lista e sua quantidade para a biblioteca encontrar estes widgets.
myDisplay.setTextButton(arrayTextButton,qtdTextButton);
```

`nullptr` significa “não liguei nada aqui”. Por exemplo, sem callback não há uma ação do seu programa para executar; sem `subtitle`, não há Label associado. Uma cor `CFK_...` é um valor de cor pronto da biblioteca. Uma fonte como `&RobotoRegular10pt7b` escolhe o desenho das letras. Para começar, conserve as fontes, os tamanhos e a ordem dos campos do exemplo.

### 4.3 O início: `setup()`

O Arduino executa `setup()` uma vez ao ligar. Ele prepara a comunicação, o display, o toque, os widgets e os componentes extras. A tarefa criada no fim continua cuidando da interface enquanto o seu `loop()` trabalha.

```cpp
void setup(){

    // Abre as mensagens de diagnóstico no computador.
    Serial.begin(115200);

    // Inicia os fios de comunicação SPI do display.
    spi_shared.begin(DISP_SCLK, DISP_MISO, DISP_MOSI);
    bus = new Arduino_HWSPI(DISP_DC, DISP_CS, DISP_SCLK, DISP_MOSI, DISP_MISO, &spi_shared);
    tft = new Arduino_ST7796(bus, DISP_RST, rotationScreen, isIPS, 320, 480);
    tft->begin(DISP_FREQUENCY);
    // Entrega à DisplayWX o objeto que desenha no display.
    myDisplay.setDrawObject(tft);

    // Informa a faixa de coordenadas recebida do toque.
    myDisplay.setTouchCorners(TOUCH_MAP_X0, TOUCH_MAP_X1, TOUCH_MAP_Y0, TOUCH_MAP_Y1);
    myDisplay.setInvertAxis(TOUCH_INVERT_X, TOUCH_INVERT_Y);
    myDisplay.setSwapAxis(TOUCH_SWAP_XY);
    // Inicia o controlador de toque escolhido em user_setup.h.
    myDisplay.startTouchFT6336(DISPLAY_W, DISPLAY_H, rotationScreen, TOUCH_SDA, TOUCH_SCL, TOUCH_INT, TOUCH_RST);

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

#### `tbssid_cb()`

Obtém o texto confirmado do nome da rede. O ponteiro se refere ao conteúdo interno da caixa.

```cpp
void tbssid_cb(){

    // Obtém o valor atual da caixa ou controle.
    tb0_val = tbssid.getValue();
}
```

#### `tbpass_cb()`

Obtém o texto confirmado da senha; não o imprime na Serial.

```cpp
void tbpass_cb(){

    // Obtém o valor atual da caixa ou controle.
    tb1_val = tbpass.getValue();
}
```

#### `textbutton1_cb()`

Só deixa um pedido de conexão para o loop, mantendo a resposta ao toque rápida. A espera da rede acontece depois.

```cpp
void textbutton1_cb(){
    connectWiFi = true;
    Serial.print("Clicked on: ");Serial.println("textbutton1_cb");
}
```

As mensagens da `Serial` servem para observar o programa; os widgets não precisam delas para desenhar.

### 4.5 O trabalho repetido: `loop()`

Confirmar cada caixa com OK faz seu callback guardar o texto. Connect deixa `connectWiFi = true`, um bilhete para o loop iniciar a tentativa. O loop espera até conectar ou completar dez segundos e atualiza a mensagem de estado.

```cpp
void loop(){

    if(connectWiFi){
        connectWiFi = false;
        WiFi.begin(tb0_val, tb1_val);
        uint32_t startTime = millis();
        while(WiFi.status() != WL_CONNECTED && millis() - startTime < timeOutConnectWiFi){
            // Pausa em milissegundos: 1000 ms equivalem a um segundo.
            delay(1000);
        }
    }

    if(WiFi.status() == WL_CONNECTED){
        Serial.println("WiFi connected");
        isWiFiConnected = true;
    }else{
        Serial.println("WiFi connection failed");
        isWiFiConnected = false;
    }

    // Reserva o desenho para esta sequência do seu programa.
    myDisplay.startCustomDraw();
    text3.setText(isWiFiConnected ? "Connected" : "Disconnected");
    // Devolve o desenho à tarefa da interface.
    myDisplay.finishCustomDraw();
    // Pausa em milissegundos: 1000 ms equivalem a um segundo.
    delay(2000);
}
```

Mantenha `startCustomDraw()` e `finishCustomDraw()` em pares, como abrir e fechar um caderno de desenho. Deixe pausas, esperas de rede e tarefas demoradas fora desse par.

### 4.6 As telas

Cada função `screen...` pinta o fundo, coloca os textos fixos e manda desenhar os widgets daquela página. Troque de página com `myDisplay.loadScreen(screen0)` ou com a função da outra tela; passe o nome sem parênteses.

```cpp
void screen0(){

    // Pinta o fundo da tela com a cor escolhida.
    myDisplay.setBackgroundColor(CFK_WHITE);
    // Escreve um texto fixo; esse texto não é um widget interativo.
    myDisplay.printText("SSID", 120, 92, TL_DATUM, CFK_BLACK, CFK_WHITE, &RobotoRegular10pt7b);

    // Escreve um texto fixo; esse texto não é um widget interativo.
    myDisplay.printText("Pass", 120, 142, TL_DATUM, CFK_BLACK, CFK_WHITE, &RobotoRegular10pt7b);

    // Escreve um texto fixo; esse texto não é um widget interativo.
    myDisplay.printText("Connect WiFi", 175, 22, TL_DATUM, CFK_BLACK, CFK_WHITE, &RobotoRegular10pt7b);
    // Desenha os widgets registrados para esse número de tela.
    myDisplay.drawWidgetsOnScreen(0);
}
```

### 4.7 Funções extras

Este exemplo não precisa de outras funções próprias além das que acabamos de estudar. As funções como `digitalWrite`, `delay` e `analogRead` pertencem ao ambiente Arduino.

## 5. Métodos dos widgets para usar no seu programa

Um **método** é uma ação que pertence a um objeto. Em `objeto.metodo(valor)`, primeiro vem o nome da peça, depois o ponto, a ação e suas informações entre parênteses. As tabelas usam objetos reais deste exemplo. Faça essas chamadas **depois da configuração**; a linha `setup(...)` pertence à preparação. Os nomes das fichas só existem dentro de `loadWidgets()`.

### Label: métodos para seu programa

Uma etiqueta de texto. Mostra palavras ou números com prefixo e sufixo, mas não recebe toques.

| Método | Como usar | Resultado e cuidado |
| --- | --- | --- |
| `setup(config)` | `text3.setup(configLabel0);` | Aplica a ficha de configuração após preparar o display. Não retorna valor. Configure no `loadWidgets()` antes de iniciar a tarefa. |
| `setText(texto)` | `text3.setText("Pronto");` | Mostra texto de `const char*` ou `String`, montando prefixo + texto + sufixo. Não retorna valor. |
| `setPrefix(texto)` | `text3.setPrefix("Valor: ");` | Guarda o texto que vem antes. Chame depois `setText`, `setTextInt` ou `setTextFloat` para reconstruir o conteúdo exibido. |
| `setSuffix(texto)` | `text3.setSuffix(" V");` | Guarda o texto que vem depois. Atualize o conteúdo em seguida para mostrar a nova combinação. |
| `setTextInt(valor)` | `text3.setTextInt(42);` | Mostra um número inteiro com prefixo e sufixo. |
| `setTextFloat(valor, casas = 2)` | `text3.setTextFloat(3.25, 2);` | Mostra número decimal. O segundo argumento escolhe 0 a 5 casas; omitido, usa 2. |
| `setDecimalPlaces(casas)` | `text3.setDecimalPlaces(1);` | Guarda a precisão, limitada a 0–5. Não reformata sozinho o texto; `setTextFloat` define novamente a precisão com seu próprio argumento. |
| `setFontSize(escala)` | `text3.setFontSize(2);` | Define o fator de tamanho das letras; use 1 para normal. Não pede atualização sozinho: atualize o texto ou recarregue a tela depois. |
| `show()` | `text3.show();` | Torna visível na sua tela e pede atualização. Para TouchArea, volta a permitir toque; ela continua invisível. |
| `hide()` | `text3.hide();` | Impede futuras atualizações visuais e, quando há toque, sua interação. Não apaga os pixels já desenhados: repinte a tela como explicado abaixo. |

Para limpar prefixo ou sufixo, use `""`. Altere os dois antes de mandar o novo conteúdo. Um Label não mede nada: é o programa que fornece o valor.

### TextBox: métodos para seu programa

Uma caixa para escrever palavras usando o teclado da própria tela.

| Método | Como usar | Resultado e cuidado |
| --- | --- | --- |
| `setup(config)` | `tbssid.setup(configTextBox0);` | Aplica a ficha de configuração após preparar o display. Não retorna valor. Configure no `loadWidgets()` antes de iniciar a tarefa. |
| `onClick()` | `tbssid.onClick();` | Abre o teclado pelo programa. Não recebe argumento para controlar callback. |
| `setValue(texto)` | `tbssid.setValue("MinhaRede");` | Copia o texto e pede atualização da caixa. Não chama o callback. |
| `getValue()` | `const char* texto = tbssid.getValue();` | Retorna o texto interno. Não o altere. Use `String copia = tbssid.getValue();` se precisar de uma cópia independente. |
| `setEnabled(valor)` | `tbssid.setEnabled(false);` | Método herdado: bloqueia o toque com false e permite com true. Não altera o valor nem executa callback. |
| `isEnabled()` | `bool ativo = tbssid.isEnabled();` | Consulta a habilitação para toque. |
| `lock()` | `tbssid.lock();` | Bloqueia a interação por toque enquanto você não chamar unlock(). Não é uma trava para organizar tarefas do processador. |
| `unlock()` | `tbssid.unlock();` | Remove esse bloqueio de toque; não desfaz setEnabled(false). |
| `isLocked()` | `bool bloqueado = tbssid.isLocked();` | Consulta se lock() está ativo. |
| `show()` | `tbssid.show();` | Torna visível na sua tela e pede atualização. Para TouchArea, volta a permitir toque; ela continua invisível. |
| `hide()` | `tbssid.hide();` | Impede futuras atualizações visuais e, quando há toque, sua interação. Não apaga os pixels já desenhados: repinte a tela como explicado abaixo. |

Toque na caixa, use `CLR` para limpar e confirme com `OK`. `CAP` muda maiúsculas/minúsculas; `DEL` apaga. A caixa não possui modo de senha: os caracteres ficam visíveis. O callback recebe a confirmação da edição; escrever por `setValue()` não o dispara. Existe construtor sem argumentos, mas aqui usamos posição e tela.

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

### Esconder e mostrar na tela

`hide()` não funciona como uma borracha para os pixels que já foram pintados. Para remover o desenho da página atual, esconda a peça e peça que a página seja montada novamente. Para fazê-la voltar, use `show()`. O estado/valor continua guardado.

```cpp
// Execute uma vez, na tela 0 deste exemplo.
text3.hide();
myDisplay.loadScreen(screen0);

// Em outra ação, quando quiser mostrar de novo:
text3.show();
myDisplay.loadScreen(screen0);
```

Os dois blocos são ações separadas: se executar todos de uma vez, o widget voltará logo em seguida. Textos fixos feitos com `printText()` continuam aparecendo, pois pertencem à função de tela. Um widget da tela 1 só aparece quando você volta à tela 1.

O desenho periódico e a detecção de toque ficam a cargo da DisplayWX. Seu `.ino` configura, registra, consulta e altera as peças pelas chamadas acima; não precisa chamar rotinas internas de desenho ou de detecção.

## 6. Experiência opcional

Ao preencher pelo código, atualize também a variável usada por WiFi.begin, pois setValue não executa o callback de confirmação.

```cpp
tbssid.setValue("MinhaRede");
tb0_val = tbssid.getValue();
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

1. Edite e confirme o SSID e a senha.
2. Toque Connect e aguarde a tentativa, que pode levar até dez segundos mais a pausa do loop.
3. Confira Status: Connected. Se aparecer Disconnected, revise dados e sinal e tente novamente.

| Se acontecer... | Confira... |
| --- | --- |
| Erro ao compilar sobre tela, toque ou tipo desconhecido | Placa selecionada, versões instaladas e opções ativas no `user_setup.h` da raiz da biblioteca. Copie o programa inteiro. |
| Tela apagada ou sem desenho | Alimentação, GND, iluminação do módulo e fios/conectores do display. |
| Desenho correto, mas o toque não responde ou está deslocado | Controlador selecionado, SDA/SCL e configuração de rotação, limites e inversão dos eixos. Use os valores completos do exemplo para essa montagem. |
| Valor parece demorar para mudar | Confira o `delay()` no loop. Vários exemplos só enviam uma atualização a cada dois segundos. |
| Método com argumentos diferentes dá erro | Consulte a tabela da classe exata. Nem todos os `onClick` ou `setValue` recebem os mesmos argumentos. |

Este manual foi conferido com os arquivos locais da biblioteca. A validação descrita aqui é de código e documentação; a compilação para a sua placa e o ensaio físico devem ser feitos com a montagem indicada.

### Arquivos usados como referência

- [textbox_textbutton_wifi.ino](textbox_textbutton_wifi.ino).
- Label: [declarações `.h`](../../../src/widgets/label/wlabel.h) e [implementação `.cpp`](../../../src/widgets/label/wlabel.cpp).
- TextBox: [declarações `.h`](../../../src/widgets/textbox/wtextbox.h) e [implementação `.cpp`](../../../src/widgets/textbox/wtextbox.cpp).
- TextButton: [declarações `.h`](../../../src/widgets/textbutton/wtextbutton.h) e [implementação `.cpp`](../../../src/widgets/textbutton/wtextbutton.cpp).
- [DisplayWX](../../../src/displaywx.h) e [implementação da integração](../../../src/displaywx.cpp).
- [WidgetBase](../../../src/widgets/widgetbase.h) e [implementação](../../../src/widgets/widgetbase.cpp), para habilitação e bloqueio herdados.

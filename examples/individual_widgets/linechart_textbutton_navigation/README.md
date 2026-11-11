# LineChart e TextButton: gráfico em duas telas

Um **widget** é uma peça da tela, como um botão, uma barra ou uma etiqueta. Você acompanha um sinal em um gráfico, abre uma segunda tela e depois volta. O gráfico continua recebendo pontos enquanto você navega.

Este manual acompanha [linechart_textbutton_navigation.ino](linechart_textbutton_navigation.ino). A referência é o código local da **DisplayWX 1.0.33**, incluindo os `.h` e `.cpp` atuais dos widgets.

![Tela do exemplo: LineChart e TextButton: gráfico em duas telas.](screenshot.png)

*Imagem que acompanha o exemplo. Ela ilustra a disposição das peças; valores, cores ativas e textos podem mudar durante a execução. Confira os estados iniciais descritos abaixo.*

![Segunda tela do exemplo, com o controle para voltar.](<screenshot (1).png>)

*Segunda página do mesmo exemplo.*

## Caminho da leitura

1. [Entenda o que cada peça faz](#1-o-que-este-exemplo-faz).
2. [Prepare a placa, os fios e as bibliotecas](#2-prepare-a-placa-e-as-bibliotecas).
3. [Copie o programa completo](#3-programa-completo).
4. [Aprenda os objetos, as fichas, os callbacks e o loop](#4-o-programa-explicado-por-partes).
5. [Consulte os métodos disponíveis para seu programa](#5-métodos-dos-widgets-para-usar-no-seu-programa).
6. [Faça uma experiência](#6-experiência-opcional).
7. [Confira o resultado](#7-como-conferir-se-funcionou).

## 1. O que este exemplo faz

O loop lê a entrada e acrescenta um ponto a cada ciclo, com pausa de 10 ms. O botão Next escolhe `screen1`; Click here volta para `screen0`. `workInBackground = true` permite guardar pontos mesmo na segunda tela.

| Peça no programa | Widget | Sua tarefa |
| --- | --- | --- |
| `labelchart11` | Label | Legenda da última amostra. |
| `linechart1` | LineChart | Gráfico da série 0 na primeira tela. |
| `btnnext` | TextButton | Abre a tela 1. |
| `btnbackpage` | TextButton | Retorna à tela 0. |

**Label:** Uma etiqueta de texto. Mostra palavras ou números com prefixo e sufixo, mas não recebe toques.

**LineChart:** Um gráfico que guarda uma sequência de números e liga os pontos. Cada série é uma linha independente.

**TextButton:** Um botão com palavras escritas dentro. Ele executa uma ação, como somar, navegar ou conectar.

O intervalo de 10 ms é uma pausa do programa, não uma garantia de taxa exata de amostragem. Este exemplo não substitui um osciloscópio. O gráfico tem memória limitada aos pontos que cabem na área.

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

Para começar, use um potenciômetro de 10 kΩ entre 3,3 V e GND, com terminal central no GPIO 16. Um gerador de sinais só pode ser conectado com saída limitada a 0–3,3 V e GND comum, sem tensões negativas.

| Nome no programa | GPIO no código abaixo | Ligação |
| --- | --- | --- |
| `pinAdcFrequencyGenerator` | 16 | Entrada analógica de teste 0–3,3 V |

Confira que esses GPIOs estão expostos e livres na sua placa. GPIOs 17 e 18 ficam reservados ao toque nesta montagem.

### Como carregar

1. Copie a pasta inteira deste exemplo para uma pasta de trabalho. Mantenha o nome da pasta igual ao nome do `.ino`.
2. Abra o `.ino` na IDE Arduino e substitua seu conteúdo pelo **programa completo abaixo**. Mantenha os arquivos `.h` que acompanham o exemplo.
3. Configure o `user_setup.h`, a placa e a porta como indicado acima.
4. Confira os fios, clique em **Verificar** e depois em **Carregar**.
5. Abra o Monitor Serial em **115200 baud** e reinicie a placa para acompanhar o início. Se estiver usando a USB nativa do ESP32-S3, a opção **USB CDC On Boot**, quando disponível para sua placa, precisa estar habilitada para os diagnósticos nessa porta.

### Ajustes desta versão didática

O programa abaixo mantém os widgets e a proposta do `.ino`, com estes ajustes explícitos para a montagem descrita:

- A inicialização dos pinos ou da Serial2 vem antes de `createTask()`, para que os callbacks encontrem o hardware pronto. As saídas começam em LOW quando não havia estado inicial explícito.

O `.ino` original da pasta continua disponível no link do início. Para usar as ligações deste manual, copie esta versão completa.

## 3. Programa completo

Copie o bloco inteiro, da primeira linha à última chave. Ele inclui a preparação da tela e todas as funções usadas. Os trechos isolados da próxima seção explicam este mesmo programa; não devem ser acrescentados novamente.

```cpp
#if CONFIG_IDF_TARGET_ESP32S2 || CONFIG_IDF_TARGET_ESP32S3
#define VSPI FSPI
#endif

#include <SPI.h>
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
void screen1();
void loadWidgets();

void btnnext_cb();
void btnbackpage_cb();

#if defined(CONFIG_IDF_TARGET_ESP32S3)
SPIClass spi_shared(FSPI);
#else
SPIClass spi_shared(HSPI);
#endif
Arduino_DataBus *bus = nullptr;
Arduino_GFX *tft = nullptr;
DisplayWX myDisplay;

// labelchart11: Legenda da última amostra.
Label labelchart11(170, 225, 0);
const uint8_t qtdLabel = 1;
Label *arrayLabel[qtdLabel] = {&labelchart11};
// linechart1: Gráfico da série 0 na primeira tela.
LineChart linechart1(90, 80, 0);
const uint8_t qtdLineChart = 1;
LineChart *arrayLinechart[qtdLineChart] = {&linechart1};
Label *seriesGrafico0[1] = {&labelchart11};
const uint8_t qtdLinesChart0 = 1;
uint16_t colorsChart0[qtdLinesChart0] = {CFK_COLOR08};
// btnnext: Abre a tela 1.
TextButton btnnext(370, 15, 0);
// btnbackpage: Retorna à tela 0.
TextButton btnbackpage(125, 150, 1);
const uint8_t qtdTextButton = 2;
TextButton *arrayTextButton[qtdTextButton] = {&btnnext, &btnbackpage};

const int pinAdcFrequencyGenerator = 16;

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

    pinMode(pinAdcFrequencyGenerator, INPUT);
    // Inicia o trabalho da biblioteca que atende o toque e atualiza a tela.
    myDisplay.createTask(false, 3);

}

void loop(){

    int valueAdc = analogRead(pinAdcFrequencyGenerator);
    int mappedValue = map(valueAdc, 0, 4095, 0, 100);

    myDisplay.startCustomDraw();
    linechart1.push(0, mappedValue);
    myDisplay.finishCustomDraw();
    delay(10);
}

void screen0(){

    myDisplay.setBackgroundColor(CFK_WHITE);
    tft->fillRect(66, 207, 313, 56, CFK_GREY3);
    tft->drawRect(66, 207, 313, 56, CFK_BLACK);

    myDisplay.drawWidgetsOnScreen(0);
}
void screen1(){

    myDisplay.setBackgroundColor(CFK_WHITE);
    myDisplay.printText("Click on button to back screen", 110, 87, TL_DATUM, CFK_BLACK, CFK_WHITE, &RobotoRegular10pt7b);

    myDisplay.drawWidgetsOnScreen(1);
}

void loadWidgets(){

    LabelConfig configLabel0 = {
            .text = "label_chart 1",
            .prefix = "Line",
            .suffix = "",
            .fontFamily = &RobotoRegular10pt7b,
            .datum = TL_DATUM,
            .fontColor = CFK_COLOR08,
            .backgroundColor = CFK_GREY3
        };
    // Aplica a ficha ao objeto; faça isso durante a preparação.
    labelchart11.setup(configLabel0);
    myDisplay.setLabel(arrayLabel,qtdLabel);

    LineChartConfig configLineChart0 = {
            .colorsSeries = colorsChart0,
            .subtitles = seriesGrafico0,
            .font = &RobotoRegular5pt7b,
            .minValue = 0,
            .maxValue = 100,
            .width = 258,
            .height = 119,
            .gridColor = CFK_BLACK,
            .borderColor = CFK_BLACK,
            .backgroundColor = CFK_GREY3,
            .textColor = CFK_WHITE,
            .verticalDivision = 5,
            .maxPointsAmount = LineChart::SHOW_ALL,
            .amountSeries = qtdLinesChart0,
            .workInBackground = true,
            .showZeroLine = true,
            .boldLine = false,
            .showDots = false
        };
    // Aplica a ficha ao objeto; faça isso durante a preparação.
    linechart1.setup(configLineChart0);
    myDisplay.setLineChart(arrayLinechart,qtdLineChart);

    TextButtonConfig configTextButton0 = {
            .text = "Next",
            // Função chamada quando o usuário aciona este controle.
            .callback = btnnext_cb,
            .fontFamily = &RobotoBold8pt7b,
            .width = 97,
            .height = 38,
            .radius = 10,
            .backgroundColor = CFK_COLOR24,
            .textColor = CFK_WHITE
        };

    // Aplica a ficha ao objeto; faça isso durante a preparação.
    btnnext.setup(configTextButton0);
    TextButtonConfig configTextButton1 = {
            .text = "Click here",
            // Função chamada quando o usuário aciona este controle.
            .callback = btnbackpage_cb,
            .fontFamily = &RobotoBold10pt7b,
            .width = 243,
            .height = 38,
            .radius = 10,
            .backgroundColor = CFK_GREY3,
            .textColor = CFK_WHITE
        };

    // Aplica a ficha ao objeto; faça isso durante a preparação.
    btnbackpage.setup(configTextButton1);
    myDisplay.setTextButton(arrayTextButton,qtdTextButton);

}

void btnnext_cb(){

    Serial.print("Clicked on: ");Serial.println("btnnext_cb");
    myDisplay.loadScreen(screen1);
}

void btnbackpage_cb(){

    Serial.print("Clicked on: ");Serial.println("btnbackpage_cb");
    myDisplay.loadScreen(screen0);
}
```

## 4. O programa explicado por partes

### 4.1 Objetos, variáveis e listas

Um **objeto** é uma peça que seu programa pode chamar pelo nome. Uma **variável** é uma caixinha que guarda um valor. `bool` guarda `true` ou `false`; `int` guarda um inteiro; `float` e `double` guardam números com parte decimal. `const` indica algo que não será trocado durante a execução.


```cpp
// labelchart11: Legenda da última amostra.
Label labelchart11(170, 225, 0);
// Quantidade de objetos desta lista.
const uint8_t qtdLabel = 1;
// Lista dos endereços dos widgets que a DisplayWX vai cuidar.
Label *arrayLabel[qtdLabel] = {&labelchart11};
// linechart1: Gráfico da série 0 na primeira tela.
LineChart linechart1(90, 80, 0);
// Quantidade de objetos desta lista.
const uint8_t qtdLineChart = 1;
// Lista dos endereços dos widgets que a DisplayWX vai cuidar.
LineChart *arrayLinechart[qtdLineChart] = {&linechart1};
Label *seriesGrafico0[1] = {&labelchart11};
// Quantidade de objetos desta lista.
const uint8_t qtdLinesChart0 = 1;
uint16_t colorsChart0[qtdLinesChart0] = {CFK_COLOR08};
// btnnext: Abre a tela 1.
TextButton btnnext(370, 15, 0);
// btnbackpage: Retorna à tela 0.
TextButton btnbackpage(125, 150, 1);
// Quantidade de objetos desta lista.
const uint8_t qtdTextButton = 2;
// Lista dos endereços dos widgets que a DisplayWX vai cuidar.
TextButton *arrayTextButton[qtdTextButton] = {&btnnext, &btnbackpage};

const int pinAdcFrequencyGenerator = 16;
```

| Objeto | Como interpretar a criação |
| --- | --- |
| `labelchart11` (Label) | X = 170, Y = 225, tela = 0. Referência da posição: ponto de alinhamento escolhido por datum. |
| `linechart1` (LineChart) | X = 90, Y = 80, tela = 0. Referência da posição: canto superior esquerdo. |
| `btnnext` (TextButton) | X = 370, Y = 15, tela = 0. Referência da posição: canto superior esquerdo. |
| `btnbackpage` (TextButton) | X = 125, Y = 150, tela = 1. Referência da posição: canto superior esquerdo. |

X cresce para a direita e Y cresce para baixo, a partir do canto superior esquerdo da tela. O número da tela decide em qual página a peça aparece. A primeira tela é 0.

O `&` antes do nome entrega o **endereço** do objeto, como informar onde fica uma casa. O `*` na declaração da lista indica que ela guarda endereços. `qtd...` precisa corresponder à quantidade de itens: ao acrescentar um widget, acrescente seu endereço e ajuste a quantidade. Mantenha os objetos e essas listas globais, fora das funções, para continuarem existindo enquanto a interface trabalha.

### 4.2 A ficha de configuração (`struct`)

A `struct` funciona como uma ficha: cada campo escolhe uma característica. O ponto em `.width` significa “preencha o campo largura”. Depois, `objeto.setup(ficha)` entrega a ficha ao widget. Configure cada peça uma vez em `loadWidgets()`, depois de preparar o display e antes de iniciar a tarefa.

#### `configLabel0`: ficha de `labelchart11`

```cpp
LabelConfig configLabel0 = {
            .text = "label_chart 1",
            .prefix = "Line",
            .suffix = "",
            .fontFamily = &RobotoRegular10pt7b,
            .datum = TL_DATUM,
            .fontColor = CFK_COLOR08,
            .backgroundColor = CFK_GREY3
        };
// Aplica a ficha ao objeto; faça isso durante a preparação.
labelchart11.setup(configLabel0);
```

| Campo | Valor nesta ficha | Para que serve |
| --- | --- | --- |
| `text` | `"label_chart 1"` | Texto inicial. Escreva entre aspas, por exemplo `"Olá"`. |
| `prefix` | `"Line"` | Texto colocado antes do valor, como `"Valor: "`. `""` deixa vazio. |
| `suffix` | `""` | Texto colocado depois do valor, como `" V"`. `""` deixa vazio. |
| `fontFamily` | `&RobotoRegular10pt7b` | Fonte das letras, por exemplo `&RobotoRegular10pt7b`. O `&` aponta para uma fonte que já existe. |
| `datum` | `TL_DATUM` | Alinhamento do texto em relação a `(x,y)`: `TL_DATUM` usa o canto superior esquerdo; `TC_DATUM`, o centro do topo. |
| `fontColor` | `CFK_COLOR08` | Cor das letras, usando os nomes `CFK_...`. |
| `backgroundColor` | `CFK_GREY3` | Cor do fundo, por exemplo `CFK_WHITE`. Use a cor da área onde o widget aparece. |

Legenda da última amostra.

#### `configLineChart0`: ficha de `linechart1`

```cpp
LineChartConfig configLineChart0 = {
            .colorsSeries = colorsChart0,
            // Lista de Labels: um para cada linha do gráfico.
            .subtitles = seriesGrafico0,
            .font = &RobotoRegular5pt7b,
            .minValue = 0,
            .maxValue = 100,
            .width = 258,
            .height = 119,
            .gridColor = CFK_BLACK,
            .borderColor = CFK_BLACK,
            .backgroundColor = CFK_GREY3,
            .textColor = CFK_WHITE,
            .verticalDivision = 5,
            .maxPointsAmount = LineChart::SHOW_ALL,
            .amountSeries = qtdLinesChart0,
            // Permite receber dados mesmo em outra tela.
            .workInBackground = true,
            .showZeroLine = true,
            .boldLine = false,
            .showDots = false
        };
// Aplica a ficha ao objeto; faça isso durante a preparação.
linechart1.setup(configLineChart0);
```

| Campo | Valor nesta ficha | Para que serve |
| --- | --- | --- |
| `colorsSeries` | `colorsChart0` | Lista de cores das linhas do gráfico, uma cor por série. |
| `subtitles` | `seriesGrafico0` | Lista de endereços dos Labels, na ordem das séries; `nullptr` deixa sem legendas. |
| `font` | `&RobotoRegular5pt7b` | Fonte das letras. O `&` antes do nome aponta para a fonte escolhida. |
| `minValue` | `0` | Menor valor da escala. Escolha um número menor que `maxValue`. |
| `maxValue` | `100` | Maior valor da escala. Escolha um número maior que `minValue`. |
| `width` | `258` | Largura em pixels, os pontinhos da imagem. Mantenha o desenho dentro da tela. |
| `height` | `119` | Altura em pixels. Reserve espaço suficiente para o desenho e as letras. |
| `gridColor` | `CFK_BLACK` | Cor da grade do gráfico. |
| `borderColor` | `CFK_BLACK` | Cor da borda. |
| `backgroundColor` | `CFK_GREY3` | Cor do fundo, por exemplo `CFK_WHITE`. Use a cor da área onde o widget aparece. |
| `textColor` | `CFK_WHITE` | Cor das letras/números. |
| `verticalDivision` | `5` | Quantidade de divisões verticais da escala do gráfico. |
| `maxPointsAmount` | `LineChart::SHOW_ALL` | Quantidade de pontos guardados por linha, pelo menos 2. `LineChart::SHOW_ALL` usa o espaço disponível; não significa memória infinita. |
| `amountSeries` | `qtdLinesChart0` | Quantidade de linhas independentes do gráfico, de 1 a 10. |
| `workInBackground` | `true` | `true` aceita pontos mesmo em outra tela; `false` só aceita quando a tela do gráfico está selecionada. |
| `showZeroLine` | `true` | Pede uma linha de referência em zero, quando aplicável à escala. |
| `boldLine` | `false` | `true` engrossa as linhas; `false` usa linhas finas. |
| `showDots` | `false` | `true` pede pontos nas amostras; o espaço entre elas precisa permitir esse desenho. |

Gráfico da série 0 na primeira tela.

#### `configTextButton0`: ficha de `btnnext`

```cpp
TextButtonConfig configTextButton0 = {
            .text = "Next",
            // Função chamada quando o usuário aciona este controle.
            .callback = btnnext_cb,
            .fontFamily = &RobotoBold8pt7b,
            .width = 97,
            .height = 38,
            .radius = 10,
            .backgroundColor = CFK_COLOR24,
            .textColor = CFK_WHITE
        };
// Aplica a ficha ao objeto; faça isso durante a preparação.
btnnext.setup(configTextButton0);
```

| Campo | Valor nesta ficha | Para que serve |
| --- | --- | --- |
| `text` | `"Next"` | Texto inicial. Escreva entre aspas, por exemplo `"Olá"`. |
| `callback` | `btnnext_cb` | Nome da função chamada quando há uma ação. Escreva o nome sem `()`. `nullptr` significa sem callback. |
| `fontFamily` | `&RobotoBold8pt7b` | Fonte das letras, por exemplo `&RobotoRegular10pt7b`. O `&` aponta para uma fonte que já existe. |
| `width` | `97` | Largura em pixels, os pontinhos da imagem. Mantenha o desenho dentro da tela. |
| `height` | `38` | Altura em pixels. Reserve espaço suficiente para o desenho e as letras. |
| `radius` | `10` | Raio do arredondamento dos cantos, não o tamanho do botão. |
| `backgroundColor` | `CFK_COLOR24` | Cor do fundo, por exemplo `CFK_WHITE`. Use a cor da área onde o widget aparece. |
| `textColor` | `CFK_WHITE` | Cor das letras/números. |

Abre a tela 1.

#### `configTextButton1`: ficha de `btnbackpage`

```cpp
TextButtonConfig configTextButton1 = {
            .text = "Click here",
            // Função chamada quando o usuário aciona este controle.
            .callback = btnbackpage_cb,
            .fontFamily = &RobotoBold10pt7b,
            .width = 243,
            .height = 38,
            .radius = 10,
            .backgroundColor = CFK_GREY3,
            .textColor = CFK_WHITE
        };
// Aplica a ficha ao objeto; faça isso durante a preparação.
btnbackpage.setup(configTextButton1);
```

Retorna à tela 0.

Após configurar, registre cada lista. É assim que a DisplayWX sabe quais peças precisa acompanhar:

```cpp
// Registra a lista e sua quantidade para a biblioteca encontrar estes widgets.
myDisplay.setLabel(arrayLabel,qtdLabel);
// Registra a lista e sua quantidade para a biblioteca encontrar estes widgets.
myDisplay.setLineChart(arrayLinechart,qtdLineChart);
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

    // OUTPUT comanda uma saída; INPUT lê um sinal externo.
    pinMode(pinAdcFrequencyGenerator, INPUT);
    // Inicia o trabalho da biblioteca que atende o toque e atualiza a tela.
    myDisplay.createTask(false, 3);

}
```

As declarações como `void screen0();` no começo do programa são **avisos de que essas funções existem**. Elas permitem usar seus nomes nas fichas antes de chegar à parte que contém o corpo da função. Não apague esses avisos ao copiar trechos.

### 4.4 Callbacks: o que acontece após a ação

Um **callback** é a tarefa que você escolhe para acontecer quando um controle é acionado ou uma edição é confirmada. Na ficha, escreva o nome **sem `()`**. A biblioteca o chama no momento adequado. Evite esperas longas dentro dele: guarde um pedido em uma variável e faça a parte demorada no `loop()`, quando necessário.

#### `btnnext_cb()`

Abre a tela 1. A mensagem na Serial ajuda a conferir que a ação aconteceu.

```cpp
void btnnext_cb(){

    Serial.print("Clicked on: ");Serial.println("btnnext_cb");
    // Pede a troca de tela; passe o nome da função, sem ().
    myDisplay.loadScreen(screen1);
}
```

#### `btnbackpage_cb()`

Retorna à tela 0. A mensagem na Serial ajuda a conferir que a ação aconteceu.

```cpp
void btnbackpage_cb(){

    Serial.print("Clicked on: ");Serial.println("btnbackpage_cb");
    // Pede a troca de tela; passe o nome da função, sem ().
    myDisplay.loadScreen(screen0);
}
```

As mensagens da `Serial` servem para observar o programa; os widgets não precisam delas para desenhar.

### 4.5 O trabalho repetido: `loop()`

O loop lê a entrada e acrescenta um ponto a cada ciclo, com pausa de 10 ms. O botão Next escolhe `screen1`; Click here volta para `screen0`. `workInBackground = true` permite guardar pontos mesmo na segunda tela.

```cpp
void loop(){

    // Lê a entrada analógica: nesta configuração, de 0 a 4095.
    int valueAdc = analogRead(pinAdcFrequencyGenerator);
    // Transforma a leitura de uma escala em outra, usando números inteiros.
    int mappedValue = map(valueAdc, 0, 4095, 0, 100);

    // Reserva o desenho para esta sequência do seu programa.
    myDisplay.startCustomDraw();
    // Acrescenta uma amostra à série indicada do gráfico.
    linechart1.push(0, mappedValue);
    // Devolve o desenho à tarefa da interface.
    myDisplay.finishCustomDraw();
    // Pausa em milissegundos: 1000 ms equivalem a um segundo.
    delay(10);
}
```

Mantenha `startCustomDraw()` e `finishCustomDraw()` em pares, como abrir e fechar um caderno de desenho. Deixe pausas, esperas de rede e tarefas demoradas fora desse par.

`analogRead()` fornece uma contagem. `map()` troca a escala por uma conta: por exemplo, 0–4095 pode virar 0–100. Não transforma o widget em um sensor; a leitura continua vindo do pino físico.

### 4.6 As telas

Cada função `screen...` pinta o fundo, coloca os textos fixos e manda desenhar os widgets daquela página. Troque de página com `myDisplay.loadScreen(screen0)` ou com a função da outra tela; passe o nome sem parênteses.

```cpp
void screen0(){

    // Pinta o fundo da tela com a cor escolhida.
    myDisplay.setBackgroundColor(CFK_WHITE);
    tft->fillRect(66, 207, 313, 56, CFK_GREY3);
    tft->drawRect(66, 207, 313, 56, CFK_BLACK);

    // Desenha os widgets registrados para esse número de tela.
    myDisplay.drawWidgetsOnScreen(0);
}
```


```cpp
void screen1(){

    // Pinta o fundo da tela com a cor escolhida.
    myDisplay.setBackgroundColor(CFK_WHITE);
    // Escreve um texto fixo; esse texto não é um widget interativo.
    myDisplay.printText("Click on button to back screen", 110, 87, TL_DATUM, CFK_BLACK, CFK_WHITE, &RobotoRegular10pt7b);

    // Desenha os widgets registrados para esse número de tela.
    myDisplay.drawWidgetsOnScreen(1);
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
| `setup(config)` | `labelchart11.setup(configLabel0);` | Aplica a ficha de configuração após preparar o display. Não retorna valor. Configure no `loadWidgets()` antes de iniciar a tarefa. |
| `setText(texto)` | `labelchart11.setText("Pronto");` | Mostra texto de `const char*` ou `String`, montando prefixo + texto + sufixo. Não retorna valor. |
| `setPrefix(texto)` | `labelchart11.setPrefix("Valor: ");` | Guarda o texto que vem antes. Chame depois `setText`, `setTextInt` ou `setTextFloat` para reconstruir o conteúdo exibido. |
| `setSuffix(texto)` | `labelchart11.setSuffix(" V");` | Guarda o texto que vem depois. Atualize o conteúdo em seguida para mostrar a nova combinação. |
| `setTextInt(valor)` | `labelchart11.setTextInt(42);` | Mostra um número inteiro com prefixo e sufixo. |
| `setTextFloat(valor, casas = 2)` | `labelchart11.setTextFloat(3.25, 2);` | Mostra número decimal. O segundo argumento escolhe 0 a 5 casas; omitido, usa 2. |
| `setDecimalPlaces(casas)` | `labelchart11.setDecimalPlaces(1);` | Guarda a precisão, limitada a 0–5. Não reformata sozinho o texto; `setTextFloat` define novamente a precisão com seu próprio argumento. |
| `setFontSize(escala)` | `labelchart11.setFontSize(2);` | Define o fator de tamanho das letras; use 1 para normal. Não pede atualização sozinho: atualize o texto ou recarregue a tela depois. |
| `show()` | `labelchart11.show();` | Torna visível na sua tela e pede atualização. Para TouchArea, volta a permitir toque; ela continua invisível. |
| `hide()` | `labelchart11.hide();` | Impede futuras atualizações visuais e, quando há toque, sua interação. Não apaga os pixels já desenhados: repinte a tela como explicado abaixo. |

Para limpar prefixo ou sufixo, use `""`. Altere os dois antes de mandar o novo conteúdo. Um Label não mede nada: é o programa que fornece o valor.

### LineChart: métodos para seu programa

Um gráfico que guarda uma sequência de números e liga os pontos. Cada série é uma linha independente.

| Método | Como usar | Resultado e cuidado |
| --- | --- | --- |
| `setup(config)` | `linechart1.setup(configLineChart0);` | Aplica a ficha de configuração após preparar o display. Não retorna valor. Configure no `loadWidgets()` antes de iniciar a tarefa. |
| `push(indiceDaSerie, valor)` | `bool aceito = linechart1.push(0, 50);` | Adiciona um ponto à série; a primeira é 0. Retorna `true` se aceitou ou `false` se não pôde. Limita o valor à escala. Os pontos mais antigos dão lugar aos novos. |
| `show()` | `linechart1.show();` | Torna visível na sua tela e pede atualização. Para TouchArea, volta a permitir toque; ela continua invisível. |
| `hide()` | `linechart1.hide();` | Impede futuras atualizações visuais e, quando há toque, sua interação. Não apaga os pixels já desenhados: repinte a tela como explicado abaixo. |

O índice vai de 0 a `amountSeries - 1`. Envie um ponto para cada série quando quiser amostras alinhadas. `workInBackground` permite guardar pontos em outra tela; `hide()` impede o desenho, mas não é uma pausa da coleta. Não existe `getValue()` nesta classe. Os Labels de `subtitles` mostram os últimos valores quando o gráfico é atualizado.

### TextButton: métodos para seu programa

Um botão com palavras escritas dentro. Ele executa uma ação, como somar, navegar ou conectar.

| Método | Como usar | Resultado e cuidado |
| --- | --- | --- |
| `setup(config)` | `btnnext.setup(configTextButton0);` | Aplica a ficha de configuração após preparar o display. Não retorna valor. Configure no `loadWidgets()` antes de iniciar a tarefa. |
| `onClick()` | `btnnext.onClick();` | Solicita o callback pelo código. Não recebe argumento `triggerCallback` e não guarda estado ligado/desligado. |
| `getEnabled()` | `bool podeTocar = btnnext.getEnabled();` | Retorna se o botão está habilitado para toque. |
| `setEnabled(valor)` | `btnnext.setEnabled(false);` | `false` bloqueia o toque e `true` permite. Não muda automaticamente a cor; `onClick()` pelo código continua possível. |
| `show()` | `btnnext.show();` | Torna visível na sua tela e pede atualização. Para TouchArea, volta a permitir toque; ela continua invisível. |
| `hide()` | `btnnext.hide();` | Impede futuras atualizações visuais e, quando há toque, sua interação. Não apaga os pixels já desenhados: repinte a tela como explicado abaixo. |

O `radius` da configuração arredonda os cantos; não é o tamanho do botão. Use dimensões maiores que as letras. Para trocar o texto inicial, altere `.text` e carregue o programa novamente; não existe `setText()` nesta classe. Toques prolongados podem repetir ações.

### Esconder e mostrar na tela

`hide()` não funciona como uma borracha para os pixels que já foram pintados. Para remover o desenho da página atual, esconda a peça e peça que a página seja montada novamente. Para fazê-la voltar, use `show()`. O estado/valor continua guardado.

```cpp
// Execute uma vez, na tela 0 deste exemplo.
labelchart11.hide();
myDisplay.loadScreen(screen0);

// Em outra ação, quando quiser mostrar de novo:
labelchart11.show();
myDisplay.loadScreen(screen0);
```

Os dois blocos são ações separadas: se executar todos de uma vez, o widget voltará logo em seguida. Textos fixos feitos com `printText()` continuam aparecendo, pois pertencem à função de tela. Um widget da tela 1 só aparece quando você volta à tela 1.

O desenho periódico e a detecção de toque ficam a cargo da DisplayWX. Seu `.ino` configura, registra, consulta e altera as peças pelas chamadas acima; não precisa chamar rotinas internas de desenho ou de detecção.

## 6. Experiência opcional

Troque temporariamente a chamada do loop para observar uma linha constante em 50, inclusive após voltar de outra tela.

```cpp
linechart1.push(0, 50);
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

1. Na tela 0, gire o potenciômetro e acompanhe os pontos.
2. Toque Next e observe a tela de retorno.
3. Mude a entrada e toque Click here: o gráfico deve mostrar os pontos recentes coletados durante a navegação.

| Se acontecer... | Confira... |
| --- | --- |
| Erro ao compilar sobre tela, toque ou tipo desconhecido | Placa selecionada, versões instaladas e opções ativas no `user_setup.h` da raiz da biblioteca. Copie o programa inteiro. |
| Tela apagada ou sem desenho | Alimentação, GND, iluminação do módulo e fios/conectores do display. |
| Desenho correto, mas o toque não responde ou está deslocado | Controlador selecionado, SDA/SCL e configuração de rotação, limites e inversão dos eixos. Use os valores completos do exemplo para essa montagem. |
| Tela funciona, mas o componente externo não acompanha | Use os GPIOs da tabela deste manual, GND comum e a polaridade correta. Uma mudança com callback desativado pode atualizar só o widget. |
| Valor parece demorar para mudar | Confira o `delay()` no loop. Vários exemplos só enviam uma atualização a cada dois segundos. |
| Método com argumentos diferentes dá erro | Consulte a tabela da classe exata. Nem todos os `onClick` ou `setValue` recebem os mesmos argumentos. |

Este manual foi conferido com os arquivos locais da biblioteca. A validação descrita aqui é de código e documentação; a compilação para a sua placa e o ensaio físico devem ser feitos com a montagem indicada.

### Arquivos usados como referência

- [linechart_textbutton_navigation.ino](linechart_textbutton_navigation.ino).
- Label: [declarações `.h`](../../../src/widgets/label/wlabel.h) e [implementação `.cpp`](../../../src/widgets/label/wlabel.cpp).
- LineChart: [declarações `.h`](../../../src/widgets/linechart/wlinechart.h) e [implementação `.cpp`](../../../src/widgets/linechart/wlinechart.cpp).
- TextButton: [declarações `.h`](../../../src/widgets/textbutton/wtextbutton.h) e [implementação `.cpp`](../../../src/widgets/textbutton/wtextbutton.cpp).
- [DisplayWX](../../../src/displaywx.h) e [implementação da integração](../../../src/displaywx.cpp).

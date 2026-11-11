# CheckBox: três caixas para comandar saídas

Um **widget** é uma peça da tela, como um botão, uma barra ou uma etiqueta. Você vai marcar e desmarcar três caixas na tela. Cada uma controla uma saída diferente, como três interruptores independentes.

Este manual acompanha [checkbox_relays.ino](checkbox_relays.ino). A referência é o código local da **DisplayWX 1.0.33**, incluindo os `.h` e `.cpp` atuais dos widgets.

![Tela do exemplo: CheckBox: três caixas para comandar saídas.](screenshot.png)

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

Um toque troca a marca da caixa. O callback correspondente lê `getStatus()` e escreve `HIGH` ou `LOW` no GPIO. `true` vira HIGH e `false` vira LOW. O `loop()` fica vazio porque as ações acontecem nos callbacks.

| Peça no programa | Widget | Sua tarefa |
| --- | --- | --- |
| `checkbox1` | CheckBox | Comanda a saída do relé 1. |
| `checkbox2` | CheckBox | Comanda a saída do relé 2. |
| `checkbox3` | CheckBox | Comanda a saída do relé 3. |

**CheckBox:** Uma caixinha que pode ficar marcada ou desmarcada. Cada caixa decide seu próprio estado.

O código assume entradas de relé ativas em HIGH. Se seu módulo aciona em LOW, a lógica precisa ser invertida para esse módulo. A marca da caixa é o estado escolhido pelo programa; não é uma leitura de confirmação do contato do relé.

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

Para o primeiro teste, use três LEDs, cada um com resistor de 330 Ω em série: GPIO → resistor → anodo do LED; catodo → GND. Depois, as saídas podem ir às entradas de módulos de relé compatíveis com lógica de 3,3 V, com alimentação própria adequada e GND comum. A bobina de um relé não vai diretamente ao GPIO. Faça o exercício com cargas de baixa tensão, sem rede elétrica.

| Nome no programa | GPIO no `.ino` e neste manual | Ligação |
| --- | --- | --- |
| `pinRelay1` | 16 | Entrada IN1 do módulo, ou LED de teste 1 |
| `pinRelay2` | 21 | Entrada IN2 do módulo, ou LED de teste 2 |
| `pinRelay3` | 47 | Entrada IN3 do módulo, ou LED de teste 3 |

Os GPIOs desta tabela já estão definidos no `.ino` e nos trechos de código deste manual. Os pinos do display, da iluminação e do toque foram preservados; GPIOs 17 e 18 ficam reservados ao toque nesta montagem. Confira que os GPIOs dos periféricos estão expostos e livres na sua placa.

### Como carregar

1. Copie a pasta inteira deste exemplo para uma pasta de trabalho. Mantenha o nome da pasta igual ao nome do `.ino`.
2. Abra o `.ino` na IDE Arduino e substitua seu conteúdo pelo **programa completo abaixo**. Mantenha os arquivos `.h` que acompanham o exemplo.
3. Configure o `user_setup.h`, a placa e a porta como indicado acima.
4. Confira os fios, clique em **Verificar** e depois em **Carregar**.
5. Abra o Monitor Serial em **115200 baud** e reinicie a placa para acompanhar o início. Se estiver usando a USB nativa do ESP32-S3, a opção **USB CDC On Boot**, quando disponível para sua placa, precisa estar habilitada para os diagnósticos nessa porta.

### Ajustes desta versão didática

O programa abaixo mantém os widgets e a proposta do `.ino`, com estes ajustes explícitos para a montagem descrita:

- A inicialização dos pinos ou da Serial2 vem antes de `createTask()`, para que os callbacks encontrem o hardware pronto. As saídas começam em LOW quando não havia estado inicial explícito.

As ligações são as mesmas no `.ino` e neste manual. Para reproduzir também os ajustes didáticos acima, copie o programa completo a seguir.

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
void loadWidgets();

void checkbox1_cb();
void checkbox2_cb();
void checkbox3_cb();

#if defined(CONFIG_IDF_TARGET_ESP32S3)
SPIClass spi_shared(FSPI);
#else
SPIClass spi_shared(HSPI);
#endif
Arduino_DataBus *bus = nullptr;
Arduino_GFX *tft = nullptr;
DisplayWX myDisplay;

// checkbox1: Comanda a saída do relé 1.
CheckBox checkbox1(90, 80, 0);
// checkbox2: Comanda a saída do relé 2.
CheckBox checkbox2(90, 130, 0);
// checkbox3: Comanda a saída do relé 3.
CheckBox checkbox3(90, 180, 0);
const uint8_t qtdCheckbox = 3;
CheckBox *arrayCheckbox[qtdCheckbox] = {&checkbox1, &checkbox2, &checkbox3};

const int pinRelay1 = 16;
const int pinRelay2 = 21;
const int pinRelay3 = 47;

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

    pinMode(pinRelay1, OUTPUT);
    pinMode(pinRelay2, OUTPUT);
    pinMode(pinRelay3, OUTPUT);
    digitalWrite(pinRelay1, LOW);
    digitalWrite(pinRelay2, LOW);
    digitalWrite(pinRelay3, LOW);
    // Inicia o trabalho da biblioteca que atende o toque e atualiza a tela.
    myDisplay.createTask(false, 3);

}

void loop(){

}

void screen0(){

    myDisplay.setBackgroundColor(CFK_WHITE);

    myDisplay.printText("Relay 1", 120, 82, TL_DATUM, CFK_COLOR04, CFK_WHITE, &RobotoRegular10pt7b);
    myDisplay.printText("Relay 2", 120, 132, TL_DATUM, CFK_COLOR20, CFK_WHITE, &RobotoRegular10pt7b);
    myDisplay.printText("Relay 3", 120, 182, TL_DATUM, CFK_COLOR01, CFK_WHITE, &RobotoRegular10pt7b);
    myDisplay.drawWidgetsOnScreen(0);
}

void loadWidgets(){

    CheckBoxConfig configCheckbox0 = {
            // Função chamada quando o usuário aciona este controle.
            .callback = checkbox1_cb,
            .weight = CheckBoxWeight::MEDIUM,
            .size = 21,
            .checkedColor = CFK_COLOR04,
            .uncheckedColor = CFK_GREY10
        };
    // Aplica a ficha ao objeto; faça isso durante a preparação.
    checkbox1.setup(configCheckbox0);
    CheckBoxConfig configCheckbox1 = {
            // Função chamada quando o usuário aciona este controle.
            .callback = checkbox2_cb,
            .weight = CheckBoxWeight::MEDIUM,
            .size = 21,
            .checkedColor = CFK_COLOR20,
            .uncheckedColor = CFK_GREY10
        };
    // Aplica a ficha ao objeto; faça isso durante a preparação.
    checkbox2.setup(configCheckbox1);
    CheckBoxConfig configCheckbox2 = {
            // Função chamada quando o usuário aciona este controle.
            .callback = checkbox3_cb,
            .weight = CheckBoxWeight::MEDIUM,
            .size = 21,
            .checkedColor = CFK_COLOR30,
            .uncheckedColor = CFK_GREY10
        };
    // Aplica a ficha ao objeto; faça isso durante a preparação.
    checkbox3.setup(configCheckbox2);
    myDisplay.setCheckbox(arrayCheckbox,qtdCheckbox);

}

void checkbox1_cb(){

    bool myValue = checkbox1.getStatus();
    Serial.print("New value for checkbox is: ");Serial.println(myValue);
    digitalWrite(pinRelay1, myValue ? HIGH : LOW);
}

void checkbox2_cb(){

    bool myValue = checkbox2.getStatus();
    Serial.print("New value for checkbox is: ");Serial.println(myValue);
    digitalWrite(pinRelay2, myValue ? HIGH : LOW);
}

void checkbox3_cb(){

    bool myValue = checkbox3.getStatus();
    Serial.print("New value for checkbox is: ");Serial.println(myValue);
    digitalWrite(pinRelay3, myValue ? HIGH : LOW);
}
```

## 4. O programa explicado por partes

### 4.1 Objetos, variáveis e listas

Um **objeto** é uma peça que seu programa pode chamar pelo nome. Uma **variável** é uma caixinha que guarda um valor. `bool` guarda `true` ou `false`; `int` guarda um inteiro; `float` e `double` guardam números com parte decimal. `const` indica algo que não será trocado durante a execução.


```cpp
// checkbox1: Comanda a saída do relé 1.
CheckBox checkbox1(90, 80, 0);
// checkbox2: Comanda a saída do relé 2.
CheckBox checkbox2(90, 130, 0);
// checkbox3: Comanda a saída do relé 3.
CheckBox checkbox3(90, 180, 0);
// Quantidade de objetos desta lista.
const uint8_t qtdCheckbox = 3;
// Lista dos endereços dos widgets que a DisplayWX vai cuidar.
CheckBox *arrayCheckbox[qtdCheckbox] = {&checkbox1, &checkbox2, &checkbox3};

const int pinRelay1 = 16;
const int pinRelay2 = 21;
const int pinRelay3 = 47;
```

| Objeto | Como interpretar a criação |
| --- | --- |
| `checkbox1` (CheckBox) | X = 90, Y = 80, tela = 0. Referência da posição: canto superior esquerdo. |
| `checkbox2` (CheckBox) | X = 90, Y = 130, tela = 0. Referência da posição: canto superior esquerdo. |
| `checkbox3` (CheckBox) | X = 90, Y = 180, tela = 0. Referência da posição: canto superior esquerdo. |

X cresce para a direita e Y cresce para baixo, a partir do canto superior esquerdo da tela. O número da tela decide em qual página a peça aparece. A primeira tela é 0.

O `&` antes do nome entrega o **endereço** do objeto, como informar onde fica uma casa. O `*` na declaração da lista indica que ela guarda endereços. `qtd...` precisa corresponder à quantidade de itens: ao acrescentar um widget, acrescente seu endereço e ajuste a quantidade. Mantenha os objetos e essas listas globais, fora das funções, para continuarem existindo enquanto a interface trabalha.

### 4.2 A ficha de configuração (`struct`)

A `struct` funciona como uma ficha: cada campo escolhe uma característica. O ponto em `.width` significa “preencha o campo largura”. Depois, `objeto.setup(ficha)` entrega a ficha ao widget. Configure cada peça uma vez em `loadWidgets()`, depois de preparar o display e antes de iniciar a tarefa.

#### `configCheckbox0`: ficha de `checkbox1`

```cpp
CheckBoxConfig configCheckbox0 = {
            // Função chamada quando o usuário aciona este controle.
            .callback = checkbox1_cb,
            .weight = CheckBoxWeight::MEDIUM,
            .size = 21,
            .checkedColor = CFK_COLOR04,
            .uncheckedColor = CFK_GREY10
        };
// Aplica a ficha ao objeto; faça isso durante a preparação.
checkbox1.setup(configCheckbox0);
```

| Campo | Valor nesta ficha | Para que serve |
| --- | --- | --- |
| `callback` | `checkbox1_cb` | Nome da função chamada quando há uma ação. Escreva o nome sem `()`. `nullptr` significa sem callback. |
| `weight` | `CheckBoxWeight::MEDIUM` | Espessura do símbolo de marcação: `CheckBoxWeight::LIGHT`, `MEDIUM` ou `HEAVY`. |
| `size` | `21` | Lado do quadrado em pixels. No CheckBox, use de 10 a 100. |
| `checkedColor` | `CFK_COLOR04` | Cor da caixa marcada. |
| `uncheckedColor` | `CFK_GREY10` | Cor da caixa desmarcada. O valor `0` escolhe a cor automática do tema. |

Comanda a saída do relé 1.

#### `configCheckbox1`: ficha de `checkbox2`

```cpp
CheckBoxConfig configCheckbox1 = {
            // Função chamada quando o usuário aciona este controle.
            .callback = checkbox2_cb,
            .weight = CheckBoxWeight::MEDIUM,
            .size = 21,
            .checkedColor = CFK_COLOR20,
            .uncheckedColor = CFK_GREY10
        };
// Aplica a ficha ao objeto; faça isso durante a preparação.
checkbox2.setup(configCheckbox1);
```

Comanda a saída do relé 2.

#### `configCheckbox2`: ficha de `checkbox3`

```cpp
CheckBoxConfig configCheckbox2 = {
            // Função chamada quando o usuário aciona este controle.
            .callback = checkbox3_cb,
            .weight = CheckBoxWeight::MEDIUM,
            .size = 21,
            .checkedColor = CFK_COLOR30,
            .uncheckedColor = CFK_GREY10
        };
// Aplica a ficha ao objeto; faça isso durante a preparação.
checkbox3.setup(configCheckbox2);
```

Comanda a saída do relé 3.

Após configurar, registre cada lista. É assim que a DisplayWX sabe quais peças precisa acompanhar:

```cpp
// Registra a lista e sua quantidade para a biblioteca encontrar estes widgets.
myDisplay.setCheckbox(arrayCheckbox,qtdCheckbox);
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
    pinMode(pinRelay1, OUTPUT);
    // OUTPUT comanda uma saída; INPUT lê um sinal externo.
    pinMode(pinRelay2, OUTPUT);
    // OUTPUT comanda uma saída; INPUT lê um sinal externo.
    pinMode(pinRelay3, OUTPUT);
    // Comanda uma saída física: HIGH liga o sinal; LOW o desliga.
    digitalWrite(pinRelay1, LOW);
    // Comanda uma saída física: HIGH liga o sinal; LOW o desliga.
    digitalWrite(pinRelay2, LOW);
    // Comanda uma saída física: HIGH liga o sinal; LOW o desliga.
    digitalWrite(pinRelay3, LOW);
    // Inicia o trabalho da biblioteca que atende o toque e atualiza a tela.
    myDisplay.createTask(false, 3);

}
```

As declarações como `void screen0();` no começo do programa são **avisos de que essas funções existem**. Elas permitem usar seus nomes nas fichas antes de chegar à parte que contém o corpo da função. Não apague esses avisos ao copiar trechos.

### 4.4 Callbacks: o que acontece após a ação

Um **callback** é a tarefa que você escolhe para acontecer quando um controle é acionado ou uma edição é confirmada. Na ficha, escreva o nome **sem `()`**. A biblioteca o chama no momento adequado. Evite esperas longas dentro dele: guarde um pedido em uma variável e faça a parte demorada no `loop()`, quando necessário.

#### `checkbox1_cb()`

Comanda a saída do relé 1. Lê o estado escolhido; `true` significa ligado/marcado e `false`, desligado/desmarcado. Escreve o nível da saída física. `HIGH` é nível alto e `LOW`, baixo. A mensagem na Serial ajuda a conferir que a ação aconteceu.

```cpp
void checkbox1_cb(){

    // Lê o estado ligado/marcado que o widget guardou.
    bool myValue = checkbox1.getStatus();
    Serial.print("New value for checkbox is: ");Serial.println(myValue);
    // Comanda uma saída física: HIGH liga o sinal; LOW o desliga.
    digitalWrite(pinRelay1, myValue ? HIGH : LOW);
}
```

#### `checkbox2_cb()`

Comanda a saída do relé 2. Lê o estado escolhido; `true` significa ligado/marcado e `false`, desligado/desmarcado. Escreve o nível da saída física. `HIGH` é nível alto e `LOW`, baixo. A mensagem na Serial ajuda a conferir que a ação aconteceu.

```cpp
void checkbox2_cb(){

    // Lê o estado ligado/marcado que o widget guardou.
    bool myValue = checkbox2.getStatus();
    Serial.print("New value for checkbox is: ");Serial.println(myValue);
    // Comanda uma saída física: HIGH liga o sinal; LOW o desliga.
    digitalWrite(pinRelay2, myValue ? HIGH : LOW);
}
```

#### `checkbox3_cb()`

Comanda a saída do relé 3. Lê o estado escolhido; `true` significa ligado/marcado e `false`, desligado/desmarcado. Escreve o nível da saída física. `HIGH` é nível alto e `LOW`, baixo. A mensagem na Serial ajuda a conferir que a ação aconteceu.

```cpp
void checkbox3_cb(){

    // Lê o estado ligado/marcado que o widget guardou.
    bool myValue = checkbox3.getStatus();
    Serial.print("New value for checkbox is: ");Serial.println(myValue);
    // Comanda uma saída física: HIGH liga o sinal; LOW o desliga.
    digitalWrite(pinRelay3, myValue ? HIGH : LOW);
}
```

Quando aparecer `condicao ? HIGH : LOW`, leia assim: “se a condição for verdadeira, use HIGH; se for falsa, use LOW”. As mensagens da `Serial` servem para observar o programa; os widgets não precisam delas para desenhar.

### 4.5 O trabalho repetido: `loop()`

Um toque troca a marca da caixa. O callback correspondente lê `getStatus()` e escreve `HIGH` ou `LOW` no GPIO. `true` vira HIGH e `false` vira LOW. O `loop()` fica vazio porque as ações acontecem nos callbacks.

```cpp
void loop(){

}
```

O corpo vazio é intencional: a tarefa da DisplayWX continua atendendo o toque. Não é necessário consultar o toque manualmente dentro do loop.

### 4.6 As telas

Cada função `screen...` pinta o fundo, coloca os textos fixos e manda desenhar os widgets daquela página. Troque de página com `myDisplay.loadScreen(screen0)` ou com a função da outra tela; passe o nome sem parênteses.

```cpp
void screen0(){

    // Pinta o fundo da tela com a cor escolhida.
    myDisplay.setBackgroundColor(CFK_WHITE);

    // Escreve um texto fixo; esse texto não é um widget interativo.
    myDisplay.printText("Relay 1", 120, 82, TL_DATUM, CFK_COLOR04, CFK_WHITE, &RobotoRegular10pt7b);
    // Escreve um texto fixo; esse texto não é um widget interativo.
    myDisplay.printText("Relay 2", 120, 132, TL_DATUM, CFK_COLOR20, CFK_WHITE, &RobotoRegular10pt7b);
    // Escreve um texto fixo; esse texto não é um widget interativo.
    myDisplay.printText("Relay 3", 120, 182, TL_DATUM, CFK_COLOR01, CFK_WHITE, &RobotoRegular10pt7b);
    // Desenha os widgets registrados para esse número de tela.
    myDisplay.drawWidgetsOnScreen(0);
}
```

### 4.7 Funções extras

Este exemplo não precisa de outras funções próprias além das que acabamos de estudar. As funções como `digitalWrite`, `delay` e `analogRead` pertencem ao ambiente Arduino.

## 5. Métodos dos widgets para usar no seu programa

Um **método** é uma ação que pertence a um objeto. Em `objeto.metodo(valor)`, primeiro vem o nome da peça, depois o ponto, a ação e suas informações entre parênteses. As tabelas usam objetos reais deste exemplo. Faça essas chamadas **depois da configuração**; a linha `setup(...)` pertence à preparação. Os nomes das fichas só existem dentro de `loadWidgets()`.

### CheckBox: métodos para seu programa

Uma caixinha que pode ficar marcada ou desmarcada. Cada caixa decide seu próprio estado.

| Método | Como usar | Resultado e cuidado |
| --- | --- | --- |
| `setup(config)` | `checkbox1.setup(configCheckbox0);` | Aplica a ficha de configuração após preparar o display. Não retorna valor. Configure no `loadWidgets()` antes de iniciar a tarefa. |
| `getStatus()` | `bool marcado = checkbox1.getStatus();` | Retorna `true` se marcada ou `false` se desmarcada. |
| `setStatus(valor, triggerCallback = true)` | `checkbox1.setStatus(true, false);` | Marca com `true` ou desmarca com `false`. Atualiza a aparência. O segundo argumento `false` evita o callback; omitido, vale `true`. Pode chamar o callback mesmo repetindo o estado. |
| `setSize(tamanho)` | `checkbox1.setSize(30);` | Altera o lado do quadrado. Aceita 10 a 100; fora da faixa, ignora. Veja a observação sobre a marca abaixo. |
| `getSize()` | `uint16_t lado = checkbox1.getSize();` | Retorna o lado do quadrado em pixels. |
| `setColors(corMarcada, corDesmarcada = 0)` | `checkbox1.setColors(CFK_GREEN, CFK_GREY10);` | Troca as cores e pede atualização visual. `0` na segunda cor usa o tema. |
| `setEnabled(valor)` | `checkbox1.setEnabled(false);` | Método herdado: bloqueia o toque com false e permite com true. Não altera o valor nem executa callback. |
| `isEnabled()` | `bool ativo = checkbox1.isEnabled();` | Consulta a habilitação para toque. |
| `lock()` | `checkbox1.lock();` | Bloqueia a interação por toque enquanto você não chamar unlock(). Não é uma trava para organizar tarefas do processador. |
| `unlock()` | `checkbox1.unlock();` | Remove esse bloqueio de toque; não desfaz setEnabled(false). |
| `isLocked()` | `bool bloqueado = checkbox1.isLocked();` | Consulta se lock() está ativo. |
| `show()` | `checkbox1.show();` | Torna visível na sua tela e pede atualização. Para TouchArea, volta a permitir toque; ela continua invisível. |
| `hide()` | `checkbox1.hide();` | Impede futuras atualizações visuais e, quando há toque, sua interação. Não apaga os pixels já desenhados: repinte a tela como explicado abaixo. |

Nesta implementação, `setSize()` muda o quadrado, mas não recalcula os pontos do símbolo de marcação. Para mudar o tamanho mantendo a marca alinhada, altere `.size` na configuração e carregue o programa novamente. O estado inicial é desmarcado.

### Como escolher se o callback deve acontecer

Nos métodos que mostram `triggerCallback = true`, omitir o último argumento permite solicitar o callback. Passar `false` evita o callback **daquela chamada**; não apaga callbacks já pendentes. A biblioteca executa os callbacks agendados por sua tarefa, então não conte com uma variável do callback já atualizada imediatamente na linha seguinte.

Se o callback comanda um pino ou copia um valor para uma variável, uma mudança silenciosa não fará essa outra tarefa. Escolha uma chamada com callback ou atualize o pino/variável explicitamente. A regra para repetir o mesmo valor está na tabela de cada classe; não é igual em todos os widgets. Não coloque setters com callback dentro do próprio callback sem necessidade, pois isso pode criar chamadas repetidas.

### Esconder e mostrar na tela

`hide()` não funciona como uma borracha para os pixels que já foram pintados. Para remover o desenho da página atual, esconda a peça e peça que a página seja montada novamente. Para fazê-la voltar, use `show()`. O estado/valor continua guardado.

```cpp
// Execute uma vez, na tela 0 deste exemplo.
checkbox1.hide();
myDisplay.loadScreen(screen0);

// Em outra ação, quando quiser mostrar de novo:
checkbox1.show();
myDisplay.loadScreen(screen0);
```

Os dois blocos são ações separadas: se executar todos de uma vez, o widget voltará logo em seguida. Textos fixos feitos com `printText()` continuam aparecendo, pois pertencem à função de tela. Um widget da tela 1 só aparece quando você volta à tela 1.

O desenho periódico e a detecção de toque ficam a cargo da DisplayWX. Seu `.ino` configura, registra, consulta e altera as peças pelas chamadas acima; não precisa chamar rotinas internas de desenho ou de detecção.

## 6. Experiência opcional

Execute uma vez depois de a tarefa estar ativa: marca a primeira caixa e o callback aciona a saída 1. Com `setStatus(true, false)`, muda só o widget; o relé não acompanha porque a escrita no pino está no callback.

```cpp
checkbox1.setStatus(true);
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

1. Ao iniciar, as três caixas estão desmarcadas e as saídas ficam LOW na versão deste manual.
2. Toque na primeira caixa: apenas o LED da saída 1 deve acender.
3. Toque na segunda e na terceira: cada uma muda sua própria saída. Toque novamente para desligar.

| Se acontecer... | Confira... |
| --- | --- |
| Erro ao compilar sobre tela, toque ou tipo desconhecido | Placa selecionada, versões instaladas e opções ativas no `user_setup.h` da raiz da biblioteca. Copie o programa inteiro. |
| Tela apagada ou sem desenho | Alimentação, GND, iluminação do módulo e fios/conectores do display. |
| Desenho correto, mas o toque não responde ou está deslocado | Controlador selecionado, SDA/SCL e configuração de rotação, limites e inversão dos eixos. Use os valores completos do exemplo para essa montagem. |
| Tela funciona, mas o componente externo não acompanha | Use os GPIOs da tabela deste manual, GND comum e a polaridade correta. Uma mudança com callback desativado pode atualizar só o widget. |
| Método com argumentos diferentes dá erro | Consulte a tabela da classe exata. Nem todos os `onClick` ou `setValue` recebem os mesmos argumentos. |

Este manual foi conferido com os arquivos locais da biblioteca. A validação descrita aqui é de código e documentação; a compilação para a sua placa e o ensaio físico devem ser feitos com a montagem indicada.

### Arquivos usados como referência

- [checkbox_relays.ino](checkbox_relays.ino).
- CheckBox: [declarações `.h`](../../../src/widgets/checkbox/wcheckbox.h) e [implementação `.cpp`](../../../src/widgets/checkbox/wcheckbox.cpp).
- [DisplayWX](../../../src/displaywx.h) e [implementação da integração](../../../src/displaywx.cpp).
- [WidgetBase](../../../src/widgets/widgetbase.h) e [implementação](../../../src/widgets/widgetbase.cpp), para habilitação e bloqueio herdados.

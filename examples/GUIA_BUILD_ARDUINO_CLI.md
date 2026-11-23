# Configurações dos exemplos para build com Arduino CLI

Revisão estática em **19/11/2026**. Foram examinados os **82 arquivos `.ino`** existentes recursivamente nesta pasta: 58 em `display_models`, 21 em `individual_widgets` e 3 em `advanced`. Há **81 sketches com conteúdo e 1 arquivo vazio**. Nenhum build, teste ou upload foi executado nesta revisão.

O objetivo é preparar a compilação futura. As configurações abaixo não representam resultados de compilação nem certificam funcionamento no hardware. O `user_setup.h` e os códigos dos exemplos foram preservados.

## 1. Como usar este guia

1. Localize o exemplo nas tabelas da seção 5. O link abre seu `.ino`; a pasta que contém esse arquivo é a pasta a passar ao Arduino CLI.
2. Escolha o **perfil de placa** indicado na linha, usando a seção 3.
3. Configure o **`user_setup.h` da raiz da DisplayWX**, usando exatamente o conjunto de macros da linha. A seção 4 explica como fazer isso.
4. Disponibilize as dependências indicadas na seção 2 e na linha do exemplo. Mantenha os cabeçalhos que acompanham o sketch na mesma pasta.
5. Quando decidir testar, use o procedimento da seção 6. Compile um exemplo por vez, pois o `user_setup.h` é compartilhado por todos.

**O que foi determinado pelo código:** família do ESP32, classe do display, barramento, controlador de toque, sensor de câmera, dependências e arquivos auxiliares. **O que normalmente não está no `.ino`:** capacidade física da flash, tipo/capacidade da PSRAM e revisão de silício do P4. Para esses itens, este guia declara perfis de referência para build. Antes de gravar em uma placa, os valores precisam corresponder ao módulo real.

## 2. Ambiente e dependências de referência

Raiz da biblioteca: `C:\arduino_edgewx\portable\sketchbook\libraries\displaywx`.

| Componente | Versão encontrada localmente | Uso |
| --- | --- | --- |
| Arduino-ESP32, pacote `esp32:esp32` | **3.3.12** | Referência de todos os perfis deste guia |
| DisplayWX | **1.0.34** | Todos os exemplos com display gráfico; a placa do motor não a inclui |
| GFX Library for Arduino (`Arduino_GFX_Library.h`) | **1.6.5** | Exemplos com `DISP_DEFAULT` |
| Adafruit GFX Library | **1.12.1** | PCD8544 e SSD1306 |
| Adafruit BusIO | **1.17.2** | Dependência da Adafruit GFX/PCD8544 |
| Adafruit PCD8544 Nokia 5110 LCD library | **2.0.3** | Exemplos PCD8544 |
| Adafruit SSD1306 | **2.5.15** | Exemplos SSD1306 |
| U8g2 | **2.36.12** | Exemplos ST7565 com `DISP_U8G2` |
| ESP_Video | **3.3.12**, incluída no core local | Os três exemplos que iniciam câmera MIPI |
| LinkerWX | **1.7.6** | `codigo_display_7in` e `codigo_placa_motor` |
| ModbusMaster | **2.0.1** | `codigo_placa_motor` |
| TM1637TinyDisplay | **1.12.2** | `codigo_placa_motor`, com o `SEIS_DIGITOS` atual |

As versões acima foram lidas nos arquivos locais `library.properties` e `platform.txt`; preserve essas cópias para reproduzir o ambiente. O README principal ainda cita Arduino_GFX 1.5.9, mas a cópia instalada examinada é 1.6.5 e contém as classes RGB/DSI usadas pelos exemplos.

`src/extras/check_version.h` aceita Arduino-ESP32 **3.3.11 a 3.3.12**, inclusive, e rejeita outras versões com `#error`. Use **3.3.12** para esta rodada. O prefixo `Arduino1x__` dos nomes não significa que o sketch aceite o core ESP32 1.x. A placa do motor não inclui DisplayWX; ainda assim, use 3.3.12 para manter a referência de sua API de watchdog.

SPI, Wire, WiFi, FS, SD, SPIFFS, FFat, Preferences, HardwareSerial e FreeRTOS vêm do core. Os controladores de toque usados nestes exemplos estão no código da própria DisplayWX; não é necessário instalar outra biblioteca de XPT2046, FT6236, FT6336, GT911, GSL3680 ou CST820 para substituí-los. Os drivers OV5647/SC2336 estão na DisplayWX e usam a ESP_Video do core. `CameraAddon` não é incluída por esses exemplos.

## 3. Perfis de placa: menu Ferramentas e FQBN

As chaves/valores do CLI foram conferidos diretamente em:

`C:\arduino_edgewx\portable\packages\esp32\hardware\esp32\3.3.12\boards.txt`.

### 3.1 Opções que variam por perfil

| Perfil | Placa no Arduino IDE / FQBN base | CPU | Flash Mode / frequência | Flash Size | Partition Scheme | PSRAM |
| --- | --- | --- | --- | --- | --- | --- |
| **E32** | **ESP32 Dev Module** / `esp32:esp32:esp32` | `240MHz (WiFi/BT)` | `DIO`, `40MHz` | `4MB (32Mb)` | `Huge APP (3MB No OTA/1MB SPIFFS)` | `Disabled` |
| **S3** | **ESP32S3 Dev Module** / `esp32:esp32:esp32s3` | `240MHz (WiFi)` | `QIO 80MHz` | `4MB (32Mb)` | `Huge APP (3MB No OTA/1MB SPIFFS)` | `Disabled` |
| **S3-RGB** | **ESP32S3 Dev Module** / `esp32:esp32:esp32s3` | `240MHz (WiFi)` | `QIO 80MHz` | `16MB (128Mb)` | `16M Flash (3MB APP/9.9MB FATFS)` | **`OPI PSRAM`** |
| **P4** | **ESP32P4 Dev Module** / `esp32:esp32:esp32p4` | Derivada de `Chip Variant` | `QIO`, `80MHz` | `4MB (32Mb)` | `Huge APP (3MB No OTA/1MB SPIFFS)` | `Disabled` |
| **P4-DSI** | **ESP32P4 Dev Module** / `esp32:esp32:esp32p4` | Derivada de `Chip Variant` | `QIO`, `80MHz` | `16MB (128Mb)` | `16M Flash (3MB APP/9.9MB FATFS)` | **`Enabled`** |

Os perfis **E32**, **S3** e **P4** são referências de build para displays pequenos/SPI e para a placa do motor. O código não exige PSRAM nesses casos. Flash de 4 MB e partição de aplicação de 3 MB são escolhas explícitas para esta rodada, não identificação da memória física. A partição maior evita usar como padrão a aplicação de apenas 1,2 MB; o tamanho final só será conhecido quando o build for executado.

O perfil **S3-RGB** reproduz as opções de memória documentadas em [codigo_display_7in/README.md](advanced/motor_trifasico/codigo_display_7in/README.md) e na [captura da IDE](advanced/motor_trifasico/codigo_display_7in/config_ide.png). Para os demais painéis RGB, a exigência de PSRAM vem de `Arduino_ESP32RGBPanel`, que coloca o framebuffer na PSRAM; a flash de 16 MB é uma referência adotada, a confirmar na placa. **OPI PSRAM não é a opção OPI da flash**: neste perfil, a flash continua em `QIO 80MHz`.

O perfil **P4-DSI** habilita a PSRAM para os framebuffers DSI e, quando presente, a captura CSI. A flash de 16 MB é uma hipótese de build, pois os sketches não informam sua capacidade. A quantidade física de PSRAM não pode ser deduzida desses arquivos e não é escolhida por um valor em MB nesse menu.

### 3.2 Opções comuns

| Opção do menu Ferramentas | E32 | S3 e S3-RGB | P4 e P4-DSI |
| --- | --- | --- | --- |
| Upload Speed | `921600` | `921600` | `921600` |
| USB Mode | Não existe nesse perfil | `Hardware CDC and JTAG` | `Hardware CDC and JTAG` |
| USB CDC On Boot | Não existe nesse perfil | `Disabled` | `Disabled` |
| USB Firmware MSC On Boot | Não existe nesse perfil | `Disabled` | `Disabled` |
| USB DFU On Boot | Não existe nesse perfil | `Disabled` | `Disabled` |
| Upload Mode | Não existe nesse perfil | `UART0 / Hardware CDC` | `UART0 / Hardware CDC` |
| Core Debug Level | `None` / `Nenhum` | `None` / `Nenhum` | `None` / `Nenhum` |
| Arduino Runs On | `Core 1` | `Core 1` | Não há esse menu no `esp32p4` local |
| Events Run On | `Core 1` | `Core 1` | Não há esse menu no `esp32p4` local |
| Erase All Flash Before Sketch Upload | `Disabled` | `Disabled` | `Disabled` |
| JTAG Adapter | `Disabled` | `Disabled` | `Disabled` |
| Zigbee Mode | `Disabled` | `Disabled` | Não há esse menu no `esp32p4` local |
| Chip Variant | Não se aplica | Não se aplica | Ver regra abaixo |

No ESP32-S3, a frequência da flash faz parte de `FlashMode=qio`; não passe `FlashFreq=80` para esse FQBN. No ESP32-P4 local, **não há `CPUFreq`, `LoopCore` nem `EventsCore`** para esse perfil. Não copie indiscriminadamente as opções do S3 para o P4.

**Chip Variant do P4:** nenhum dos sketches identifica a revisão de silício. Os FQBNs abaixo usam explicitamente `ChipVariant=prev3` (**Before v3.00**, CPU 360 MHz) como referência. Para uma placa **v3.00 ou mais nova**, use `ChipVariant=postv3` (**v3.00 or newer**, CPU 400 MHz). Se o objetivo for testar as duas variantes, repita cada exemplo P4 com ambos os valores e registre os resultados separadamente. O nome `FUNCTIONAL`, `WAVESHARE` ou `GUITION` sozinho não comprova essa revisão.

Para logs de diagnóstico em uma rodada posterior, troque `DebugLevel=none` por `DebugLevel=info`. O README de `codigo_display_7in` recomenda Info para observar conexão e watchdog; None reproduz a captura da IDE. USB CDC On Boot Disabled mantém a receita de referência; para monitorar pela USB nativa, a escolha depende da conexão física e pode ser alterada para Enabled (`CDCOnBoot=cdc`). Porta COM e Programador não são necessários para compilar.

### 3.3 FQBNs completos para copiar

Este bloco PowerShell apenas define os perfis; não inicia compilação. A versão do core não faz parte do FQBN: confirme 3.3.12 no ambiente escolhido.

```powershell
$perfisDisplaywx = @{
    'E32' = 'esp32:esp32:esp32:CPUFreq=240,FlashMode=dio,FlashFreq=40,FlashSize=4M,PartitionScheme=huge_app,PSRAM=disabled,LoopCore=1,EventsCore=1,ZigbeeMode=default,UploadSpeed=921600,DebugLevel=none,EraseFlash=none,JTAGAdapter=default'
    'S3' = 'esp32:esp32:esp32s3:CPUFreq=240,FlashMode=qio,FlashSize=4M,PartitionScheme=huge_app,PSRAM=disabled,USBMode=hwcdc,CDCOnBoot=default,MSCOnBoot=default,DFUOnBoot=default,UploadMode=default,LoopCore=1,EventsCore=1,ZigbeeMode=default,UploadSpeed=921600,DebugLevel=none,EraseFlash=none,JTAGAdapter=default'
    'S3-RGB' = 'esp32:esp32:esp32s3:CPUFreq=240,FlashMode=qio,FlashSize=16M,PartitionScheme=app3M_fat9M_16MB,PSRAM=opi,USBMode=hwcdc,CDCOnBoot=default,MSCOnBoot=default,DFUOnBoot=default,UploadMode=default,LoopCore=1,EventsCore=1,ZigbeeMode=default,UploadSpeed=921600,DebugLevel=none,EraseFlash=none,JTAGAdapter=default'
    'P4' = 'esp32:esp32:esp32p4:ChipVariant=prev3,FlashMode=qio,FlashFreq=80,FlashSize=4M,PartitionScheme=huge_app,PSRAM=disabled,USBMode=hwcdc,CDCOnBoot=default,MSCOnBoot=default,DFUOnBoot=default,UploadMode=default,UploadSpeed=921600,DebugLevel=none,EraseFlash=none,JTAGAdapter=default'
    'P4-DSI' = 'esp32:esp32:esp32p4:ChipVariant=prev3,FlashMode=qio,FlashFreq=80,FlashSize=16M,PartitionScheme=app3M_fat9M_16MB,PSRAM=enabled,USBMode=hwcdc,CDCOnBoot=default,MSCOnBoot=default,DFUOnBoot=default,UploadMode=default,UploadSpeed=921600,DebugLevel=none,EraseFlash=none,JTAGAdapter=default'
}
```

## 4. Como preparar o `user_setup.h`

Edite o arquivo **`C:\arduino_edgewx\portable\sketchbook\libraries\displaywx\user_setup.h`**. Não é o `User_Setup.h` da TFT_eSPI e não há um arquivo separado de configuração para cada sketch.

Na revisão, o arquivo continha `DISP_DEFAULT` e `TOUCH_FT6236U`. Essa combinação serve aos exemplos ILI9488/FT6236, mas precisa ser trocada para os demais controladores.

Nas tabelas, cada nome na coluna **Macros ativas** corresponde a uma linha `#define NOME`. Use **exatamente um** `DISP_*`, **zero ou um** `TOUCH_*` e **zero ou um** `CAM_*`. Remova ou comente as demais seleções. Um `#define TOUCH_XPT2046 0` ainda conta como definido, porque a biblioteca usa `defined(...)`.

Exemplo para os widgets com ESP32-S3/ST7796/FT6336:

```cpp
#define DISP_DEFAULT
#define TOUCH_FT6336
```

Exemplo para a câmera Waveshare/OV5647:

```cpp
#define DISP_DEFAULT
#define TOUCH_GT911_WAVESHARE
#define CAM_OV5647
```

Exemplo para SSD1306 sem touch e sem câmera:

```cpp
#define DISP_SSD1306
```

Regras para interpretar as seleções:

- `DISP_DEFAULT` seleciona a interface Arduino_GFX. O modelo do painel, os pinos, os timings e a rotação continuam no construtor do `.ino`; não acrescente um `#define ILI9488` ou `ST7796` ao `user_setup.h`.
- `DISP_PCD8544`, `DISP_SSD1306` e `DISP_U8G2` substituem `DISP_DEFAULT` nos exemplos monocromáticos.
- O nome correto para o FT6236 é **`TOUCH_FT6236U`**; para o FT6336U, é **`TOUCH_FT6336`**. Use **`TOUCH_GT911_WAVESHARE`** nos dois sketches que chamam `startTouchGT911Waveshare()`.
- Ausência de `TOUCH_*` ou `CAM_*` na linha significa deixar **todas** as macros dessa categoria desativadas. Mesmo os widgets que só mostram ADC ainda chamam `startTouchFT6336()` no código atual; mantenha a macro indicada.
- Não defina manualmente `HAS_TOUCH`, `HAS_CAMERA_MIPI`, `CONFIG_IDF_TARGET_*` ou `BOARD_HAS_PSRAM`. Os dois primeiros são derivados pela DisplayWX; os demais dependem da placa/core e do menu PSRAM.
- Não basta colocar os defines no início do `.ino`: os arquivos `.cpp` e `.c` da biblioteca são compilados separadamente. Use o `user_setup.h` compartilhado para que todos recebam a mesma configuração.
- Não são necessárias alterações em `widgetsetup.h` para a rodada padrão. Preserve os widgets/fontes habilitados; desativar imagem ou câmera para reduzir o binário mudaria o que os respectivos exemplos testam.

## 5. Inventário completo por exemplo

Todos os links desta seção apontam para os arquivos existentes. **Perfil** referencia todas as opções da seção 3. **Macros ativas** é a seleção completa de display/toque/câmera. As dimensões descritas são as declaradas no sketch; divergências com o construtor estão anotadas na seção 7.

### 5.1 Exemplos avançados — 3 arquivos

| Exemplo (`.ino`) | Hardware / interface | Perfil | Macros ativas | Função, dependências e observações |
| --- | --- | --- | --- | --- |
| [controle_ambiente.ino](<advanced/controle_ambiente/controle_ambiente.ino>) | ESP32-S3; ST7796 SPI 320 × 480; FT6336U; rotação 0 | **S3** | `DISP_DEFAULT`, `TOUCH_FT6336` | Arduino_GFX; simulação AURA e fila FreeRTOS; sem sensores/rede. SDA 13/SCL 14, diferentes dos widgets individuais. |
| [codigo_display_7in.ino](<advanced/motor_trifasico/codigo_display_7in/codigo_display_7in.ino>) | ESP32-S3; ESP32-8048S070 RGB 800 × 480; GT911 | **S3-RGB** | `DISP_DEFAULT`, `TOUCH_GT911` | Arduino_GFX + LinkerWX; interface remota do motor. Manter fluxo.h, controle_conexao.h, controle_comandos.h, ipg1back.h e ipg2back.h. Memória documentada no README/captura. |
| [codigo_placa_motor.ino](<advanced/motor_trifasico/codigo_placa_motor/codigo_placa_motor.ino>) | ESP32 clássico; TM1637 de 6 dígitos | **E32** | Não se aplica: não inclui DisplayWX | LinkerWX + ModbusMaster + TM1637TinyDisplay6; manter controle_conexao.h e SEIS_DIGITOS. Wi-Fi/TLS, Modbus RTU e RPM. |

### 5.2 Widgets individuais — 21 arquivos

Os 20 exemplos sem câmera usam **ESP32-S3**, display **ST7796 via SPI**, área declarada **480 × 320**, rotação 1 e touch **FT6336U**. Todos usam o perfil **S3** e o mesmo par de macros. `camera` é a exceção: usa ESP32-P4, DSI e CSI.

| Exemplo (`.ino`) | Hardware / interface | Perfil | Macros ativas | Função, dependências e observações |
| --- | --- | --- | --- | --- |
| [camera.ino](<individual_widgets/camera/camera.ino>) | ESP32-P4; ILI9881C Waveshare 7 pol.; DSI 720 × 1280; GT911 Waveshare; CSI OV5647 | **P4-DSI** | `DISP_DEFAULT`, `TOUCH_GT911_WAVESHARE`, `CAM_OV5647` | Vídeo OV5647 em CameraMipi e botão de enquadramento; ESP_Video do core. |
| [checkbox_relays.ino](<individual_widgets/checkbox_relays/checkbox_relays.ino>) | ST7796; SPI; 480 × 320; FT6336U | **S3** | `DISP_DEFAULT`, `TOUCH_FT6336` | Três CheckBoxes acionam relés por GPIO. |
| [checkbox_vbar_label.ino](<individual_widgets/checkbox_vbar_label/checkbox_vbar_label.ino>) | ST7796; SPI; 480 × 320; FT6336U | **S3** | `DISP_DEFAULT`, `TOUCH_FT6336` | CheckBox aciona LED; ADC alimenta VBar e Label. |
| [circlebutton_stepper.ino](<individual_widgets/circlebutton_stepper/circlebutton_stepper.ino>) | ST7796; SPI; 480 × 320; FT6336U | **S3** | `DISP_DEFAULT`, `TOUCH_FT6336` | CircleButtons controlam microstepping/enable e tarefa gera pulsos STEP; sem biblioteca externa de motor. |
| [gauge_label_wifi.ino](<individual_widgets/gauge_label_wifi/gauge_label_wifi.ino>) | ST7796; SPI; 480 × 320; FT6336U | **S3** | `DISP_DEFAULT`, `TOUCH_FT6336` | Gauge/Label mostram RSSI; usa WiFi do core. |
| [hslider_rgb.ino](<individual_widgets/hslider_rgb/hslider_rgb.ino>) | ST7796; SPI; 480 × 320; FT6336U | **S3** | `DISP_DEFAULT`, `TOUCH_FT6336` | Três sliders compõem uma cor RGB desenhada na tela. |
| [image_navigation.ino](<individual_widgets/image_navigation/image_navigation.ino>) | ST7796; SPI; 480 × 320; FT6336U | **S3** | `DISP_DEFAULT`, `TOUCH_FT6336` | Imagens alternam telas; manter irightpng.h e ileftpng.h. |
| [label_voltage.ino](<individual_widgets/label_voltage/label_voltage.ino>) | ST7796; SPI; 480 × 320; FT6336U | **S3** | `DISP_DEFAULT`, `TOUCH_FT6336` | Leitura ADC convertida em tensão e exibida em Label. |
| [led_rectbutton_motor.ino](<individual_widgets/led_rectbutton_motor/led_rectbutton_motor.ino>) | ST7796; SPI; 480 × 320; FT6336U | **S3** | `DISP_DEFAULT`, `TOUCH_FT6336` | RectButton aciona motor por GPIO; LEDs mostram estado e sensores. |
| [linechart_labels.ino](<individual_widgets/linechart_labels/linechart_labels.ino>) | ST7796; SPI; 480 × 320; FT6336U | **S3** | `DISP_DEFAULT`, `TOUCH_FT6336` | Gráfico de leitura ADC e etiquetas de valores. |
| [linechart_textbutton_navigation.ino](<individual_widgets/linechart_textbutton_navigation/linechart_textbutton_navigation.ino>) | ST7796; SPI; 480 × 320; FT6336U | **S3** | `DISP_DEFAULT`, `TOUCH_FT6336` | Gráfico de sinal ADC com botões para alternar duas telas. |
| [numberbox_radiogroup_calculator.ino](<individual_widgets/numberbox_radiogroup_calculator/numberbox_radiogroup_calculator.ino>) | ST7796; SPI; 480 × 320; FT6336U | **S3** | `DISP_DEFAULT`, `TOUCH_FT6336` | Calculadora de tensão, resistência, corrente e potência. |
| [numberbox_textbutton_calculator.ino](<individual_widgets/numberbox_textbutton_calculator/numberbox_textbutton_calculator.ino>) | ST7796; SPI; 480 × 320; FT6336U | **S3** | `DISP_DEFAULT`, `TOUCH_FT6336` | Calculadora de soma/subtração com NumberBox e TextButton. |
| [radiogroup_leds.ino](<individual_widgets/radiogroup_leds/radiogroup_leds.ino>) | ST7796; SPI; 480 × 320; FT6336U | **S3** | `DISP_DEFAULT`, `TOUCH_FT6336` | RadioGroups escolhem períodos de LEDs em tarefas FreeRTOS. |
| [rectbutton_serial.ino](<individual_widgets/rectbutton_serial/rectbutton_serial.ino>) | ST7796; SPI; 480 × 320; FT6336U | **S3** | `DISP_DEFAULT`, `TOUCH_FT6336` | Dois RectButtons enviam estados por Serial2. |
| [spinbox_alarm.ino](<individual_widgets/spinbox_alarm/spinbox_alarm.ino>) | ST7796; SPI; 480 × 320; FT6336U | **S3** | `DISP_DEFAULT`, `TOUCH_FT6336` | SpinBox define limiar de ADC para saída de alarme. |
| [textbox_textbutton_wifi.ino](<individual_widgets/textbox_textbutton_wifi/textbox_textbutton_wifi.ino>) | ST7796; SPI; 480 × 320; FT6336U | **S3** | `DISP_DEFAULT`, `TOUCH_FT6336` | TextBoxes recebem rede/senha e botão solicita conexão; WiFi do core. |
| [textbutton_serial.ino](<individual_widgets/textbutton_serial/textbutton_serial.ino>) | ST7796; SPI; 480 × 320; FT6336U | **S3** | `DISP_DEFAULT`, `TOUCH_FT6336` | TextButtons enviam start/stop por Serial2. |
| [togglebutton_leds.ino](<individual_widgets/togglebutton_leds/togglebutton_leds.ino>) | ST7796; SPI; 480 × 320; FT6336U | **S3** | `DISP_DEFAULT`, `TOUCH_FT6336` | Dois ToggleButtons acionam LEDs por GPIO. |
| [toucharea_led.ino](<individual_widgets/toucharea_led/toucharea_led.ino>) | ST7796; SPI; 480 × 320; FT6336U | **S3** | `DISP_DEFAULT`, `TOUCH_FT6336` | TouchArea alterna LED físico e indicador Led na tela. |
| [vanalog_vbar_circularbar_thermometer.ino](<individual_widgets/vanalog_vbar_circularbar_thermometer/vanalog_vbar_circularbar_thermometer.ino>) | ST7796; SPI; 480 × 320; FT6336U | **S3** | `DISP_DEFAULT`, `TOUCH_FT6336` | Uma leitura ADC alimenta VAnalog, VBar, CircularBar, Thermometer e Label. |

### 5.3 Pasta `display_models/i2c` — 6 arquivos

**A localização da pasta não define o barramento:** SSD1306 usa I2C, mas PCD8544 usa SPI por software através do construtor de cinco pinos da Adafruit. Nenhum deles usa touch ou câmera.

| Exemplo (`.ino`) | Hardware / interface | Perfil | Macros ativas | Função, dependências e observações |
| --- | --- | --- | --- | --- |
| [Arduino1x__ESP32__PCD8544.ino](<display_models/i2c/esp32/Arduino1x__ESP32__PCD8544/Arduino1x__ESP32__PCD8544.ino>) | PCD8544; SPI por software; 84 × 48; sem touch | **E32** | `DISP_PCD8544` | Adafruit PCD8544 + Adafruit GFX + BusIO. |
| [Arduino1x__ESP32__SSD1306.ino](<display_models/i2c/esp32/Arduino1x__ESP32__SSD1306/Arduino1x__ESP32__SSD1306.ino>) | SSD1306; I2C; 128 × 32; sem touch | **E32** | `DISP_SSD1306` | Adafruit SSD1306 + Adafruit GFX + BusIO; endereço 0x3C. |
| [Arduino1x__ESP32P4__PCD8544.ino](<display_models/i2c/esp32p4/Arduino1x__ESP32P4__PCD8544/Arduino1x__ESP32P4__PCD8544.ino>) | PCD8544; SPI por software; 84 × 48; sem touch | **P4** | `DISP_PCD8544` | Adafruit PCD8544 + Adafruit GFX + BusIO. |
| [Arduino1x__ESP32P4__SSD1306.ino](<display_models/i2c/esp32p4/Arduino1x__ESP32P4__SSD1306/Arduino1x__ESP32P4__SSD1306.ino>) | SSD1306; I2C; 128 × 32; sem touch | **P4** | `DISP_SSD1306` | Adafruit SSD1306 + Adafruit GFX + BusIO; endereço 0x3C. |
| [Arduino1x__ESP32S3__PCD8544.ino](<display_models/i2c/esp32s3/Arduino1x__ESP32S3__PCD8544/Arduino1x__ESP32S3__PCD8544.ino>) | PCD8544; SPI por software; 84 × 48; sem touch | **S3** | `DISP_PCD8544` | Adafruit PCD8544 + Adafruit GFX + BusIO. |
| [Arduino1x__ESP32S3__SSD1306.ino](<display_models/i2c/esp32s3/Arduino1x__ESP32S3__SSD1306/Arduino1x__ESP32S3__SSD1306.ino>) | SSD1306; I2C; 128 × 32; sem touch | **S3** | `DISP_SSD1306` | Adafruit SSD1306 + Adafruit GFX + BusIO; endereço 0x3C. |

### 5.4 Pasta `display_models/spi/esp32` — 15 arquivos

Todos usam **ESP32 clássico**, placa **ESP32 Dev Module**, perfil **E32**. Os modelos `EMBED` também são ESP32 clássico. ST7565 usa U8g2/SPI por software; os demais usam Arduino_GFX.

| Exemplo (`.ino`) | Hardware / interface | Perfil | Macros ativas | Função, dependências e observações |
| --- | --- | --- | --- | --- |
| [Arduino1x__ESP32__BLACK_ST7796_35.ino](<display_models/spi/esp32/Arduino1x__ESP32__BLACK_ST7796_35/Arduino1x__ESP32__BLACK_ST7796_35.ino>) | ST7796; SPI; 480 × 320; FT6336U | **E32** | `DISP_DEFAULT`, `TOUCH_FT6336` | Arduino_GFX. |
| [Arduino1x__ESP32__GC9A01.ino](<display_models/spi/esp32/Arduino1x__ESP32__GC9A01/Arduino1x__ESP32__GC9A01.ino>) | GC9A01; SPI; 240 × 240; sem touch | **E32** | `DISP_DEFAULT` | Arduino_GFX. |
| [Arduino1x__ESP32__ILI9225_NO_TOUCH.ino](<display_models/spi/esp32/Arduino1x__ESP32__ILI9225_NO_TOUCH/Arduino1x__ESP32__ILI9225_NO_TOUCH.ino>) | ILI9225; SPI; 176 × 220; sem touch | **E32** | `DISP_DEFAULT` | Arduino_GFX. |
| [Arduino1x__ESP32__ILI9488_35_FT6236.ino](<display_models/spi/esp32/Arduino1x__ESP32__ILI9488_35_FT6236/Arduino1x__ESP32__ILI9488_35_FT6236.ino>) | ILI9488; SPI; 480 × 320; FT6236U | **E32** | `DISP_DEFAULT`, `TOUCH_FT6236U` | Arduino_GFX. |
| [Arduino1x__ESP32__ILI9488_35_XPT2046.ino](<display_models/spi/esp32/Arduino1x__ESP32__ILI9488_35_XPT2046/Arduino1x__ESP32__ILI9488_35_XPT2046.ino>) | ILI9488; SPI; 320 × 480; XPT2046 | **E32** | `DISP_DEFAULT`, `TOUCH_XPT2046` | Arduino_GFX. |
| [Arduino1x__ESP32__ILI9488_4_FT6336.ino](<display_models/spi/esp32/Arduino1x__ESP32__ILI9488_4_FT6336/Arduino1x__ESP32__ILI9488_4_FT6336.ino>) | ILI9488; SPI; 320 × 480; FT6336U | **E32** | `DISP_DEFAULT`, `TOUCH_FT6336` | Arduino_GFX. |
| [Arduino1x__ESP32__ILI9488_4_XPT2046.ino](<display_models/spi/esp32/Arduino1x__ESP32__ILI9488_4_XPT2046/Arduino1x__ESP32__ILI9488_4_XPT2046.ino>) | ILI9488; SPI; 480 × 320; XPT2046 | **E32** | `DISP_DEFAULT`, `TOUCH_XPT2046` | Arduino_GFX. |
| [Arduino1x__ESP32__ILI9488_XPT2046.ino](<display_models/spi/esp32/Arduino1x__ESP32__ILI9488_XPT2046/Arduino1x__ESP32__ILI9488_XPT2046.ino>) | ILI9488; SPI; 240 × 320; XPT2046 | **E32** | `DISP_DEFAULT`, `TOUCH_XPT2046` | Arduino_GFX. Conferir divergência de dimensões na seção 7. |
| [Arduino1x__ESP32__ST7565.ino](<display_models/spi/esp32/Arduino1x__ESP32__ST7565/Arduino1x__ESP32__ST7565.ino>) | ST7565; SPI por software; 128 × 64; sem touch | **E32** | `DISP_U8G2` | U8g2; sem touch/câmera. |
| [Arduino1x__ESP32__ST7735_144in.ino](<display_models/spi/esp32/Arduino1x__ESP32__ST7735_144in/Arduino1x__ESP32__ST7735_144in.ino>) | ST7735; SPI; 128 × 128; sem touch | **E32** | `DISP_DEFAULT` | Arduino_GFX. Conferir divergência de dimensões na seção 7. |
| [Arduino1x__ESP32__ST7735_NO_TOUCH.ino](<display_models/spi/esp32/Arduino1x__ESP32__ST7735_NO_TOUCH/Arduino1x__ESP32__ST7735_NO_TOUCH.ino>) | ST7735; SPI; 128 × 160; sem touch | **E32** | `DISP_DEFAULT` | Arduino_GFX. |
| [Arduino1x__ESP32__ST7789_240x240_NO_TOUCH.ino](<display_models/spi/esp32/Arduino1x__ESP32__ST7789_240x240_NO_TOUCH/Arduino1x__ESP32__ST7789_240x240_NO_TOUCH.ino>) | ST7789 - SPI; SPI; 240 × 240; sem touch | **E32** | `DISP_DEFAULT` | Arduino_GFX. Conferir divergência de dimensões na seção 7. |
| [Arduino1x__ESP32__ST7789_320x240_NO_TOUCH.ino](<display_models/spi/esp32/Arduino1x__ESP32__ST7789_320x240_NO_TOUCH/Arduino1x__ESP32__ST7789_320x240_NO_TOUCH.ino>) | ST7789 - SPI; SPI; 240 × 320; sem touch | **E32** | `DISP_DEFAULT` | Arduino_GFX. O código está em 240 × 320, rotação 0. |
| [Arduino1x__ESP32EMBED__GUITION_28.ino](<display_models/spi/esp32/Arduino1x__ESP32EMBED__GUITION_28/Arduino1x__ESP32EMBED__GUITION_28.ino>) | ST7789 - EMBED; SPI; 320 × 240; CST820 | **E32** | `DISP_DEFAULT`, `TOUCH_CST820` | Arduino_GFX. |
| [Arduino1x__ESP32EMBED__ST7796_YELLOW.ino](<display_models/spi/esp32/Arduino1x__ESP32EMBED__ST7796_YELLOW/Arduino1x__ESP32EMBED__ST7796_YELLOW.ino>) | ST7796; SPI; 480 × 320; XPT2046 | **E32** | `DISP_DEFAULT`, `TOUCH_XPT2046` | Arduino_GFX. |

### 5.5 Pasta `display_models/spi/esp32s3` — 18 arquivos

Todos usam **ESP32-S3**, placa **ESP32S3 Dev Module**. Os quatro exemplos com `Arduino_ESP32RGBPanel` usam perfil **S3-RGB**. `ST7789_8bit` usa barramento paralelo de 8 bits, embora esteja nesta pasta; seu código não exige o framebuffer RGB em PSRAM e recebe o perfil **S3**.

| Exemplo (`.ino`) | Hardware / interface | Perfil | Macros ativas | Função, dependências e observações |
| --- | --- | --- | --- | --- |
| [Arduino1x__ESP32S3__BLACK_ST7796_35.ino](<display_models/spi/esp32s3/Arduino1x__ESP32S3__BLACK_ST7796_35/Arduino1x__ESP32S3__BLACK_ST7796_35.ino>) | ST7796; SPI; 480 × 320; FT6336U | **S3** | `DISP_DEFAULT`, `TOUCH_FT6336` | Arduino_GFX. |
| [Arduino1x__ESP32S3__GC9A01.ino](<display_models/spi/esp32s3/Arduino1x__ESP32S3__GC9A01/Arduino1x__ESP32S3__GC9A01.ino>) | GC9A01; SPI; 240 × 240; sem touch | **S3** | `DISP_DEFAULT` | Arduino_GFX. |
| [Arduino1x__ESP32S3__ILI9225_NO_TOUCH.ino](<display_models/spi/esp32s3/Arduino1x__ESP32S3__ILI9225_NO_TOUCH/Arduino1x__ESP32S3__ILI9225_NO_TOUCH.ino>) | ILI9225; SPI; 176 × 220; sem touch | **S3** | `DISP_DEFAULT` | Arduino_GFX. |
| [Arduino1x__ESP32S3__ILI9488_35_FT6236.ino](<display_models/spi/esp32s3/Arduino1x__ESP32S3__ILI9488_35_FT6236/Arduino1x__ESP32S3__ILI9488_35_FT6236.ino>) | ILI9488; SPI; 320 × 480; FT6236U | **S3** | `DISP_DEFAULT`, `TOUCH_FT6236U` | Arduino_GFX. |
| [Arduino1x__ESP32S3__ILI9488_35_XPT2046.ino](<display_models/spi/esp32s3/Arduino1x__ESP32S3__ILI9488_35_XPT2046/Arduino1x__ESP32S3__ILI9488_35_XPT2046.ino>) | ILI9488; SPI; 320 × 480; XPT2046 | **S3** | `DISP_DEFAULT`, `TOUCH_XPT2046` | Arduino_GFX. |
| [Arduino1x__ESP32S3__ILI9488_4_FT6336.ino](<display_models/spi/esp32s3/Arduino1x__ESP32S3__ILI9488_4_FT6336/Arduino1x__ESP32S3__ILI9488_4_FT6336.ino>) | ILI9488; SPI; 320 × 480; FT6336U | **S3** | `DISP_DEFAULT`, `TOUCH_FT6336` | Arduino_GFX. |
| [Arduino1x__ESP32S3__ILI9488_4_XPT2046.ino](<display_models/spi/esp32s3/Arduino1x__ESP32S3__ILI9488_4_XPT2046/Arduino1x__ESP32S3__ILI9488_4_XPT2046.ino>) | ILI9488; SPI; 480 × 320; XPT2046 | **S3** | `DISP_DEFAULT`, `TOUCH_XPT2046` | Arduino_GFX. |
| [Arduino1x__ESP32S3__ILI9488_XPT2046.ino](<display_models/spi/esp32s3/Arduino1x__ESP32S3__ILI9488_XPT2046/Arduino1x__ESP32S3__ILI9488_XPT2046.ino>) | ILI9488; SPI; 240 × 320; XPT2046 | **S3** | `DISP_DEFAULT`, `TOUCH_XPT2046` | Arduino_GFX. Conferir divergência de dimensões na seção 7. |
| [Arduino1x__ESP32S3__ST7565.ino](<display_models/spi/esp32s3/Arduino1x__ESP32S3__ST7565/Arduino1x__ESP32S3__ST7565.ino>) | ST7565; SPI por software; 128 × 64; sem touch | **S3** | `DISP_U8G2` | U8g2; sem touch/câmera. |
| [Arduino1x__ESP32S3__ST7735_144in.ino](<display_models/spi/esp32s3/Arduino1x__ESP32S3__ST7735_144in/Arduino1x__ESP32S3__ST7735_144in.ino>) | ST7735; SPI; 128 × 128; sem touch | **S3** | `DISP_DEFAULT` | Arduino_GFX. Conferir divergência de dimensões na seção 7. |
| [Arduino1x__ESP32S3__ST7735_NO_TOUCH.ino](<display_models/spi/esp32s3/Arduino1x__ESP32S3__ST7735_NO_TOUCH/Arduino1x__ESP32S3__ST7735_NO_TOUCH.ino>) | ST7735; SPI; 128 × 160; sem touch | **S3** | `DISP_DEFAULT` | Arduino_GFX. |
| [Arduino1x__ESP32S3__ST7789_240x240_NO_TOUCH.ino](<display_models/spi/esp32s3/Arduino1x__ESP32S3__ST7789_240x240_NO_TOUCH/Arduino1x__ESP32S3__ST7789_240x240_NO_TOUCH.ino>) | ST7789 - SPI; SPI; 240 × 240; sem touch | **S3** | `DISP_DEFAULT` | Arduino_GFX. Conferir divergência de dimensões na seção 7. |
| [Arduino1x__ESP32S3__ST7789_320x240_NO_TOUCH.ino](<display_models/spi/esp32s3/Arduino1x__ESP32S3__ST7789_320x240_NO_TOUCH/Arduino1x__ESP32S3__ST7789_320x240_NO_TOUCH.ino>) | ST7789 - SPI; SPI; 240 × 320; sem touch | **S3** | `DISP_DEFAULT` | Arduino_GFX. O código está em 240 × 320, rotação 0. |
| [Arduino1x__S3EMBED__S3_4827043.ino](<display_models/spi/esp32s3/Arduino1x__S3EMBED__S3_4827043/Arduino1x__S3EMBED__S3_4827043.ino>) | ESP32-4827S043; RGB paralelo; 480 × 272; XPT2046 | **S3-RGB** | `DISP_DEFAULT`, `TOUCH_XPT2046` | Arduino_GFX. Framebuffer em PSRAM: usar S3-RGB. |
| [Arduino1x__S3EMBED__S3_8048043.ino](<display_models/spi/esp32s3/Arduino1x__S3EMBED__S3_8048043/Arduino1x__S3EMBED__S3_8048043.ino>) | ESP32-8048S043; RGB paralelo; 800 × 480; GT911 | **S3-RGB** | `DISP_DEFAULT`, `TOUCH_GT911` | Arduino_GFX. Framebuffer em PSRAM: usar S3-RGB. |
| [Arduino1x__S3EMBED__S3_8048070.ino](<display_models/spi/esp32s3/Arduino1x__S3EMBED__S3_8048070/Arduino1x__S3EMBED__S3_8048070.ino>) | ESP32-8048S070; RGB paralelo; 800 × 480; GT911 | **S3-RGB** | `DISP_DEFAULT`, `TOUCH_GT911` | Arduino_GFX. Framebuffer em PSRAM: usar S3-RGB. |
| [Arduino1x__S3EMBED__ST7789_8bit.ino](<display_models/spi/esp32s3/Arduino1x__S3EMBED__ST7789_8bit/Arduino1x__S3EMBED__ST7789_8bit.ino>) | ST7789 - 8bit; paralelo de 8 bits; 170 × 320; sem touch | **S3** | `DISP_DEFAULT` | Arduino_GFX. GPIOs e alimentação definidos no sketch; não é display SPI/RGB. |
| [Arduino1x__S3EMBED__WAVESHARE_ST7262.ino](<display_models/spi/esp32s3/Arduino1x__S3EMBED__WAVESHARE_ST7262/Arduino1x__S3EMBED__WAVESHARE_ST7262.ino>) | ST7262 - Waveshare; RGB paralelo; 800 × 480; GT911 | **S3-RGB** | `DISP_DEFAULT`, `TOUCH_GT911` | Arduino_GFX. PSRAM OPI; integração do expansor/GPIO pendente, ver seção 7. |

### 5.6 Pasta `display_models/spi/esp32p4` — 13 arquivos

Todos usam **ESP32-P4**, placa **ESP32P4 Dev Module**, perfil **P4**. São exemplos SPI, incluindo o ST7565 por software; nenhum deles habilita câmera. A revisão de silício precisa seguir a regra de `Chip Variant` da seção 3.

| Exemplo (`.ino`) | Hardware / interface | Perfil | Macros ativas | Função, dependências e observações |
| --- | --- | --- | --- | --- |
| [Arduino1x__ESP32P4__BLACK_ST7796_35.ino](<display_models/spi/esp32p4/Arduino1x__ESP32P4__BLACK_ST7796_35/Arduino1x__ESP32P4__BLACK_ST7796_35.ino>) | ST7796; SPI; 480 × 320; FT6336U | **P4** | `DISP_DEFAULT`, `TOUCH_FT6336` | Arduino_GFX. |
| [Arduino1x__ESP32P4__GC9A01.ino](<display_models/spi/esp32p4/Arduino1x__ESP32P4__GC9A01/Arduino1x__ESP32P4__GC9A01.ino>) | GC9A01; SPI; 240 × 240; sem touch | **P4** | `DISP_DEFAULT` | Arduino_GFX. |
| [Arduino1x__ESP32P4__ILI9225_NO_TOUCH.ino](<display_models/spi/esp32p4/Arduino1x__ESP32P4__ILI9225_NO_TOUCH/Arduino1x__ESP32P4__ILI9225_NO_TOUCH.ino>) | ILI9225; SPI; 176 × 220; sem touch | **P4** | `DISP_DEFAULT` | Arduino_GFX. |
| [Arduino1x__ESP32P4__ILI9488_35_FT6236.ino](<display_models/spi/esp32p4/Arduino1x__ESP32P4__ILI9488_35_FT6236/Arduino1x__ESP32P4__ILI9488_35_FT6236.ino>) | ILI9488; SPI; 480 × 320; FT6236U | **P4** | `DISP_DEFAULT`, `TOUCH_FT6236U` | Arduino_GFX. |
| [Arduino1x__ESP32P4__ILI9488_35_XPT2046.ino](<display_models/spi/esp32p4/Arduino1x__ESP32P4__ILI9488_35_XPT2046/Arduino1x__ESP32P4__ILI9488_35_XPT2046.ino>) | ILI9488; SPI; 320 × 480; XPT2046 | **P4** | `DISP_DEFAULT`, `TOUCH_XPT2046` | Arduino_GFX. |
| [Arduino1x__ESP32P4__ILI9488_4_FT6336.ino](<display_models/spi/esp32p4/Arduino1x__ESP32P4__ILI9488_4_FT6336/Arduino1x__ESP32P4__ILI9488_4_FT6336.ino>) | ILI9488; SPI; 320 × 480; FT6336U | **P4** | `DISP_DEFAULT`, `TOUCH_FT6336` | Arduino_GFX. |
| [Arduino1x__ESP32P4__ILI9488_4_XPT2046.ino](<display_models/spi/esp32p4/Arduino1x__ESP32P4__ILI9488_4_XPT2046/Arduino1x__ESP32P4__ILI9488_4_XPT2046.ino>) | ILI9488; SPI; 480 × 320; XPT2046 | **P4** | `DISP_DEFAULT`, `TOUCH_XPT2046` | Arduino_GFX. |
| [Arduino1x__ESP32P4__ILI9488_XPT2046.ino](<display_models/spi/esp32p4/Arduino1x__ESP32P4__ILI9488_XPT2046/Arduino1x__ESP32P4__ILI9488_XPT2046.ino>) | ILI9488; SPI; 240 × 320; XPT2046 | **P4** | `DISP_DEFAULT`, `TOUCH_XPT2046` | Arduino_GFX. Conferir divergência de dimensões na seção 7. |
| [Arduino1x__ESP32P4__ST7565.ino](<display_models/spi/esp32p4/Arduino1x__ESP32P4__ST7565/Arduino1x__ESP32P4__ST7565.ino>) | ST7565; SPI por software; 128 × 64; sem touch | **P4** | `DISP_U8G2` | U8g2; sem touch/câmera. |
| [Arduino1x__ESP32P4__ST7735_144in.ino](<display_models/spi/esp32p4/Arduino1x__ESP32P4__ST7735_144in/Arduino1x__ESP32P4__ST7735_144in.ino>) | ST7735; SPI; 128 × 128; sem touch | **P4** | `DISP_DEFAULT` | Arduino_GFX. Conferir divergência de dimensões na seção 7. |
| [Arduino1x__ESP32P4__ST7735_NO_TOUCH.ino](<display_models/spi/esp32p4/Arduino1x__ESP32P4__ST7735_NO_TOUCH/Arduino1x__ESP32P4__ST7735_NO_TOUCH.ino>) | ST7735; SPI; 128 × 160; sem touch | **P4** | `DISP_DEFAULT` | Arduino_GFX. |
| [Arduino1x__ESP32P4__ST7789_240x240_NO_TOUCH.ino](<display_models/spi/esp32p4/Arduino1x__ESP32P4__ST7789_240x240_NO_TOUCH/Arduino1x__ESP32P4__ST7789_240x240_NO_TOUCH.ino>) | ST7789 - SPI; SPI; 240 × 240; sem touch | **P4** | `DISP_DEFAULT` | Arduino_GFX. Conferir divergência de dimensões na seção 7. |
| [Arduino1x__ESP32P4__ST7789_320x240_NO_TOUCH.ino](<display_models/spi/esp32p4/Arduino1x__ESP32P4__ST7789_320x240_NO_TOUCH/Arduino1x__ESP32P4__ST7789_320x240_NO_TOUCH.ino>) | ST7789 - SPI; SPI; 240 × 320; sem touch | **P4** | `DISP_DEFAULT` | Arduino_GFX. O código está em 240 × 320, rotação 0. |

### 5.7 Pasta `display_models/mipi dsi/esp32p4` — 6 arquivos

Os cinco arquivos com conteúdo usam **ESP32-P4**, placa **ESP32P4 Dev Module**, perfil **P4-DSI**. Mantenha aspas ao passar esses caminhos ao CLI, pois `mipi dsi` contém um espaço.

| Exemplo (`.ino`) | Hardware / interface | Perfil | Macros ativas | Função, dependências e observações |
| --- | --- | --- | --- | --- |
| [Arduino1x__P4_WAVESHARE__DSI_7.ino](<display_models/mipi dsi/esp32p4/Arduino1x__P4_WAVESHARE__DSI_7/Arduino1x__P4_WAVESHARE__DSI_7.ino>) | ILI9881C - DSI; DSI; 720 × 1280; GT911_WAVESHARE | **P4-DSI** | `DISP_DEFAULT`, `TOUCH_GT911_WAVESHARE`, `CAM_OV5647` | Arduino_GFX. ESP_Video do core; inicia câmera OV5647. |
| [Arduino1x__P4EMBED__EYE.ino](<display_models/mipi dsi/esp32p4/Arduino1x__P4EMBED__EYE/Arduino1x__P4EMBED__EYE.ino>) | ESP32-P4 apenas inferido do nome/pasta; sem implementação | **Pendente** | Indeterminadas | Arquivo de 0 bytes; não incluir como build válido. |
| [Arduino1x__P4EMBED__FUNCTIONAL.ino](<display_models/mipi dsi/esp32p4/Arduino1x__P4EMBED__FUNCTIONAL/Arduino1x__P4EMBED__FUNCTIONAL.ino>) | EK79007 - DSI; DSI; 1024 × 600; GT911 | **P4-DSI** | `DISP_DEFAULT`, `TOUCH_GT911`, `CAM_SC2336` | Arduino_GFX. ESP_Video do core; inicia câmera SC2336. |
| [Arduino1x__P4EMBED__GUITION_DSI_4.ino](<display_models/mipi dsi/esp32p4/Arduino1x__P4EMBED__GUITION_DSI_4/Arduino1x__P4EMBED__GUITION_DSI_4.ino>) | ST7701 - DSI; DSI; 800 × 480; GT911 | **P4-DSI** | `DISP_DEFAULT`, `TOUCH_GT911` | Arduino_GFX. |
| [Arduino1x__P4EMBED__GUITION_DSI_7.ino](<display_models/mipi dsi/esp32p4/Arduino1x__P4EMBED__GUITION_DSI_7/Arduino1x__P4EMBED__GUITION_DSI_7.ino>) | JD9165 - DSI; DSI; 1024 × 600; GT911 | **P4-DSI** | `DISP_DEFAULT`, `TOUCH_GT911` | Arduino_GFX. |
| [Arduino1x__P4EMBED__WAVESHARE_DSI_10.ino](<display_models/mipi dsi/esp32p4/Arduino1x__P4EMBED__WAVESHARE_DSI_10/Arduino1x__P4EMBED__WAVESHARE_DSI_10.ino>) | JD9365 - DSI; DSI; 1280 × 800; GSL3680 | **P4-DSI** | `DISP_DEFAULT`, `TOUCH_GSL3680` | Arduino_GFX. |

## 6. Procedimento para executar futuramente

### 6.1 Selecionar o ambiente portable

`arduino-cli` não foi encontrado no PATH da sessão de revisão. No futuro, disponibilize o executável no PATH ou coloque seu caminho completo em `$cliDisplaywx`. A configuração abaixo direciona o CLI ao core e às bibliotecas desta instalação portable; as variáveis afetam apenas a sessão PowerShell. A correspondência de `directories.data` e `directories.user` segue a [documentação oficial de configuração do Arduino CLI](https://docs.arduino.cc/arduino-cli/configuration/).

```powershell
$cliDisplaywx = 'arduino-cli'
$raizDisplaywx = 'C:\arduino_edgewx\portable\sketchbook\libraries\displaywx'
$env:ARDUINO_DIRECTORIES_DATA = 'C:\arduino_edgewx\portable'
$env:ARDUINO_DIRECTORIES_USER = 'C:\arduino_edgewx\portable\sketchbook'

# Consultas para a futura sessão; não compilam sketches.
& $cliDisplaywx version
& $cliDisplaywx config dump
& $cliDisplaywx core list
& $cliDisplaywx lib list
& $cliDisplaywx board details --fqbn 'esp32:esp32:esp32'
& $cliDisplaywx board details --fqbn 'esp32:esp32:esp32s3'
& $cliDisplaywx board details --fqbn 'esp32:esp32:esp32p4'
```

Confirme o core 3.3.12 e os diretórios acima antes dos builds. A instalação do core e das dependências não foi alterada. O executável do Arduino IDE 1.x e `arduino-builder.exe` não substituem o Arduino CLI neste procedimento.

### 6.2 Compilar um exemplo

Primeiro execute o bloco de perfis da seção 3.3 e prepare manualmente o `user_setup.h`. Depois escolha o perfil e o caminho relativo da **pasta** do exemplo. O bloco a seguir é uma receita para uso futuro; não foi executado.

```powershell
# Exemplo escolhido: ESP32-S3 + ST7796 + FT6336.
# Antes de compilar, user_setup.h deve conter somente:
#   #define DISP_DEFAULT
#   #define TOUCH_FT6336
$perfilDisplaywx = 'S3'
$exemploDisplaywx = 'individual_widgets\checkbox_relays'
$pastaSketchDisplaywx = Join-Path (Join-Path $raizDisplaywx 'examples') $exemploDisplaywx
$fqbnDisplaywx = $perfisDisplaywx[$perfilDisplaywx]

# Para P4 de revisao v3.00 ou posterior, ajuste antes do build:
# $fqbnDisplaywx = $fqbnDisplaywx.Replace('ChipVariant=prev3', 'ChipVariant=postv3')

$idBuildDisplaywx = ($perfilDisplaywx + '_' + $exemploDisplaywx) -replace '[^A-Za-z0-9_-]', '_'
$pastaBuildDisplaywx = Join-Path $env:TEMP ('displaywx-build-' + $idBuildDisplaywx)
$arquivoLogDisplaywx = Join-Path $env:TEMP ('displaywx-build-' + $idBuildDisplaywx + '.log')

$argumentosDisplaywx = @(
    'compile',
    '--fqbn', $fqbnDisplaywx,
    '--libraries', 'C:\arduino_edgewx\portable\sketchbook\libraries',
    '--clean',
    '--build-path', $pastaBuildDisplaywx,
    '--warnings', 'all',
    '--verbose',
    $pastaSketchDisplaywx
)
& $cliDisplaywx @argumentosDisplaywx 2>&1 | Tee-Object -FilePath $arquivoLogDisplaywx
$resultadoBuildDisplaywx = $LASTEXITCODE
Write-Host "Codigo de saida: $resultadoBuildDisplaywx"
Write-Host "Log: $arquivoLogDisplaywx"
```

`compile` recebe a pasta do sketch e o FQBN; `--libraries` indica a coleção local de bibliotecas. `--clean` solicita recompilação sem aproveitar o build anterior, particularmente útil após trocar o `user_setup.h`. A receita não contém `--upload` e não precisa de placa conectada. Veja a [referência oficial do comando compile](https://arduino.github.io/arduino-cli/1.6/commands/arduino-cli_compile/).

Não rode builds simultâneos com seleções diferentes sobre a mesma cópia da DisplayWX. Preserve uma cópia do seu `user_setup.h` antes da rodada e restaure-a ao terminar. Para cada resultado, registre o FQBN completo, as macros, as versões efetivamente usadas e o log; no P4, diferencie também `prev3` e `postv3`. Código de saída zero indica build concluído, sem validar touch, display ou câmera físicos.

## 7. Pendências e diferenças encontradas no código

1. **`Arduino1x__P4EMBED__EYE` está vazio: 0 bytes.** Não há `setup()`, `loop()`, display, touch ou câmera a analisar. A família P4 é sugerida somente pelo caminho/nome. Não existe `user_setup` comprovável para ele; classifique-o como **pendente de implementação**, sem considerá-lo um build válido. Nenhuma configuração de menu resolve a ausência de código.
2. **O `user_setup.h` atual não serve para todos os exemplos.** A seleção FT6236U precisa ser substituída por FT6336, XPT2046, GT911 etc. conforme a tabela. A biblioteca possui verificações que rejeitam mais de um touch ou sensor de câmera ativo.
3. **Câmera também é obrigatória em dois exemplos de modelos de display.** `Arduino1x__P4_WAVESHARE__DSI_7` inicia OV5647; `Arduino1x__P4EMBED__FUNCTIONAL` inicia SC2336. Embora desenhem apenas formas/texto, ambos chamam `startCamera()`/`getCamera()`. Use as macros de câmera listadas. O terceiro caso é o widget `camera`, com OV5647 e controles de enquadramento.
4. **`Arduino1x__S3EMBED__WAVESHARE_ST7262` tem integração de hardware pendente**, conforme os comentários do próprio `.ino`: usa um expansor de I/O, mas `startTouchGT911()` trabalha com GPIO/Wire nativos. O código também usa GPIO 7 como SDA e PCLK, e GPIO 2 como backlight e RGB DATA12. Isso não se resolve escolhendo outra opção no menu; mantenha registrado para uma futura validação em placa, separada do build.
5. **Há diferenças entre nomes/dimensões declaradas e construtores.** Os três `ST7735_144in` declaram 128 × 128, mas instanciam `Arduino_ST7735` sem largura/altura, cujo padrão local é 128 × 160. Os três `ST7789_240x240_NO_TOUCH` declaram 240 × 240, mas usam o construtor padrão local de 240 × 320. Os três `ST7789_320x240_NO_TOUCH` declaram 240 × 320 e rotação 0. Os três `ILI9488_XPT2046` declaram 240 × 320, embora o driver ILI9488 tenha dimensões nativas 320 × 480. São pontos de geometria para conferir no hardware; não exigem outro FQBN ou outra macro `DISP_*` para este build.
6. **A pasta nem sempre descreve a interface real.** PCD8544 está em `i2c`, mas usa SPI por software; os painéis S3 RGB e o ST7789 paralelo estão em `spi`. As classes instanciadas no código prevaleceram na escolha dos perfis.
7. **`codigo_placa_motor` não usa DisplayWX.** O `SEIS_DIGITOS` atual seleciona `TM1637TinyDisplay6.h`, presente na biblioteca TM1637TinyDisplay instalada. Se remover esse define no futuro, o ramo alternativo exige `TM1637Display.h`, uma dependência adicional que não foi localizada na coleção examinada. Essa variante de quatro dígitos não é a configuração desta tabela.
8. **Flash, PSRAM e revisão P4 não são todas comprováveis pelos sketches.** Use os perfis assumidos para a rodada de build e registre qualquer substituição. Só `codigo_display_7in` tem neste conjunto a referência explícita de menu/memória em README e captura. Não interprete os perfis genéricos como identificação de um SKU de placa.

Os arquivos de imagens `.h` de `image_navigation` e de `codigo_display_7in` estão presentes. Compilar a pasta original preserva esses recursos. Fotos, capturas, arquivos de projeto e apresentações existentes nas pastas não são bibliotecas a instalar para o build. Os parâmetros de rede dos exemplos Wi-Fi/LinkerWX não precisam ser alterados apenas para compilar; conexão e periféricos só são usados quando o firmware executa.

## 8. Rastreabilidade da revisão

As seleções foram cruzadas entre todos os `.ino`, seus cabeçalhos locais, comentários `Project setup`, construtores de barramento/display e chamadas `startTouch*()`/`startCamera()`. Também foram examinados:

- [user_setup.h](../user_setup.h), [check_version.h](../src/extras/check_version.h), [check_touch.h](../src/check_touch.h) e [check_camera.h](../src/check_camera.h).
- [widgetsetup.h](../src/widgets/widgetsetup.h), [widgetbase.h](../src/widgets/widgetbase.h), [touch.h](../src/touch/touch.h) e [displaywx.h](../src/displaywx.h).
- [README do display de 7 polegadas](advanced/motor_trifasico/codigo_display_7in/README.md), sua [captura do menu](advanced/motor_trifasico/codigo_display_7in/config_ide.png), [README da placa do motor](advanced/motor_trifasico/codigo_placa_motor/README.md) e [README da câmera](individual_widgets/camera/README.md).
- `boards.txt`/`platform.txt` do core local 3.3.12 e os cabeçalhos/construtores da cópia local da GFX Library for Arduino.

**Estado dos builds: não executados.** As tabelas são a preparação da rodada futura.

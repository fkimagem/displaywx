# DisplayWX
Display widgets for Arduino ESP32 core **3.3.12**, using **GFX Library for Arduino 1.6.5** for the default backend. The installed core uses GCC 14.2.0, GNU C++20 and GNU C17.

[https://github.com/moononournation/Arduino_GFX]

## How to Use
1. Download all the files and unzip them.
2. Go to your `libraries` folder in the Arduino IDE (portable or Documents installation).
3. Create a new folder named `DisplayWX` and paste all the unzipped files into that folder.
4. Open your Arduino IDE (if it is already open, close and reopen it), go to **File > Examples > DisplayWX** and check that the examples are listed.

## Display backends

Select one backend in `user_setup.h`:

| Define | Drawing object | Enabled output widgets |
| --- | --- | --- |
| `DISP_DEFAULT` | `Arduino_GFX*` | All; MIPI camera additionally requires ESP32-P4, IDF >= 5.4 and a configured sensor |
| `DISP_PCD8544` | `Adafruit_PCD8544*` | Label, Led, VBar, VAnalog, GaugeSuper, LineChart, Thermometer, Image |
| `DISP_SSD1306` | `Adafruit_SSD1306*` | Same as PCD8544 |
| `DISP_ST7565` | `ST7565_LCD*` from ST7565_LCD | Same as PCD8544 |
| `DISP_U8G2` | `U8G2*` | Image |

Pass the initialized display to `DisplayWX::setDrawObject()`. Touch integration is
enabled only for `DISP_DEFAULT`. `DFK_DISABLE_IMAGE` and `DFK_DISABLE_SD` retain
their existing meanings. The UI task sends the Adafruit monochrome framebuffer
with `display()`; U8g2's buffer/page lifecycle remains managed by the sketch.

Unsupported drawing operations are excluded inside each source file, including
helpers and backend-specific types. For example, CircularBar's drawing methods
do nothing outside Arduino GFX, even when its implementation file is compiled.
This does not enable its automatic registration through `DFK_CIRCULARBAR`.

`python tests/display_backend_contract_test.py` checks conditional branches and
display method names/argument counts against the locally installed sibling
libraries. It does not compile firmware or replace a C++ type check.

## Correções e testes

Consulte [as correções, contratos de uso e roteiro de teste](docs/CORRECOES_ESP32_CORE_3.3.12.md).
A [revisão original](docs/REVISAO_TECNICA_ESP32_CORE_3.3.12.md) é um registro histórico anterior às correções.

A configuração precisa ser igual em **todas** as unidades de compilação. Edite `user_setup.h`
ou coloque esta linha em `build_opt.h` na pasta do sketch, substituindo o caminho:

```text
"-DDISPLAYWX_CONFIG_HEADER=\"C:/meu_projeto/config_displaywx.h\""
```

Mantenha as aspas e barras invertidas da linha acima: o compilador lê `build_opt.h`
como arquivo de argumentos. No caminho, use `/`. Definir opções apenas antes de
`#include <displaywx.h>` no `.ino`
não altera a compilação dos `.cpp` da biblioteca. Faça um build limpo após trocar opções.

- `DFK_COMPACT_FONTS`: inclui somente as fontes internas e um catálogo automático de 5, 8, 12 e 20 pt.
- `DFK_SELECT_FONT_FAMILIES`: ativa somente famílias `FONT_*` explicitamente definidas na configuração;
  as fontes necessárias aos widgets continuam disponíveis.
- `DFK_FONT_HEADER`: cabeçalho adicional de fontes; `DFK_ROBOTO_BOLD_CATALOG`: lista de ponteiros,
  do maior para o menor, para substituir o catálogo automático.
- `DFK_DISABLE_<recurso>`: exclui recursos opcionais; a lista e os limites de memória estão no documento.

Os algoritmos portáveis estão em `src/extras/dwx_safe.c` e `textmetrics.c`.
A API Arduino continua em C++, com guards para sincronização e destruição coordenada.

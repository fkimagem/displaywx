#ifndef WIDGETSETUP_H
#define WIDGETSETUP_H

#include <cstdint>
#include <esp_idf_version.h>
#include "soc/soc_caps.h"
#include "sdkconfig.h"
#include "../../user_setup.h"
#define INVERTE_BITS_16(x) ((uint16_t)(~(x)))

//#define DEBUG_DISPLAY_FK

#ifndef UNUSED
#define UNUSED(x) (void)x;
#endif

#define POINT_IN_RECT(px, py, rx, ry, rw, rh) \
    ((px) >= (rx) && (px) <= ((rx) + (rw)) && (py) >= (ry) && (py) <= ((ry) + (rh)))

#define POINT_IN_CIRCLE(px, py, cx, cy, r) \
    (((px) - (cx)) * ((px) - (cx)) + ((py) - (cy)) * ((py) - (cy)) <= (r) * (r))

constexpr uint16_t invert_bits(uint16_t val) {
	
#ifdef INVERT_COLORS_BITS
    return ~val;
#else
    return val;
#endif
}

#if defined(IS_BGR)
constexpr uint16_t rgb2brg(uint16_t val) {
    // troca R e B no formato RGB565
    return ((val & 0x001F) << 11) |     // B -> R
           (val & 0x07E0) |            // G mantido
           ((val & 0xF800) >> 11);     // R -> B
}
#else
constexpr uint16_t rgb2brg(uint16_t val) {
    return val;
}
#endif


#if defined(DISP_DEFAULT)
constexpr uint16_t process_color(uint16_t val) {
    return rgb2brg(invert_bits(val));
}
#elif defined(DISP_PCD8544) || defined(DISP_SSD1306)
constexpr uint16_t process_color(uint16_t val) {
    return (val == 0xFFFF) ? 0x0 : 0x1;
}
#elif defined(DISP_U8G2)
constexpr uint16_t process_color(uint16_t val) {
    return (val != 0x0000) ? 0x1 : 0x0;
}
#endif

#define DFK_TOUCHAREA 1
#define DFK_CIRCLEBTN 1
#define DFK_RECTBTN 1
#define DFK_LED 1
#define DFK_VBAR 1
#define DFK_TOGGLE 1
#define DFK_HSLIDER 1
#define DFK_TEXTBOX 1
#define DFK_CHECKBOX 1
#define DFK_RADIO 1
#define DFK_NUMBERBOX 1
#define DFK_VANALOG 1
#define DFK_GAUGE 1
#define DFK_LINECHART 1
#define DFK_LABEL 1
#if !defined(DFK_DISABLE_IMAGE)
#define DFK_IMAGE 1
#endif
#if !defined(DFK_DISABLE_SD)
#define DFK_SD 1
#endif
#define DFK_SPINBOX 1
#define DFK_TEXTBUTTON 1
#define DFK_CIRCULARBAR 1
#define DFK_THERMOMETER 1
#define DFK_CAMERA_MIPI 1
//#define DFK_EXTERNALINPUT 1

#define USE_SPIFFS 1
#define USE_FATFS 1

#if defined(DISP_PCD8544) || defined(DISP_SSD1306) || defined(DISP_U8G2)
#undef DFK_TOUCHAREA
#undef DFK_CIRCLEBTN
#undef DFK_RECTBTN
#undef DFK_TOGGLE
#undef DFK_HSLIDER
#undef DFK_TEXTBOX
#undef DFK_CHECKBOX
#undef DFK_RADIO
#undef DFK_NUMBERBOX
#undef DFK_VANALOG
#undef DFK_GAUGE
#undef DFK_LINECHART
#undef DFK_SPINBOX
#undef DFK_TEXTBUTTON
#undef DFK_CIRCULARBAR
#endif

#if defined(DISP_U8G2)
#undef DFK_LABEL
#undef DFK_THERMOMETER
#undef DFK_VBAR
#undef DFK_LED
#endif

#if defined(DISP_DEFAULT) || defined(DISP_PCD8544) || defined(DISP_SSD1306)
#define USING_GRAPHIC_LIB
#endif

 #if ESP_IDF_VERSION < ESP_IDF_VERSION_VAL(5, 4, 0)  // ESP_Video MIPI-CSI exige IDF >= 5.4
 #undef DFK_CAMERA_MIPI
 #pragma message("IDF < 5.4.0, desativando DFK_CAMERA_MIPI")
 #endif

 #if !defined(CONFIG_IDF_TARGET_ESP32P4)  // ESP_Video MIPI-CSI exige IDF >= 5.4
 #undef DFK_CAMERA_MIPI
 #pragma message("Board is not ESP32-P4, desativando DFK_CAMERA_MIPI")
 #endif

//These enumerate the text plotting alignment (reference datum point)
#define TL_DATUM 0 // Top left (default)
#define TC_DATUM 1 // Top centre
#define TR_DATUM 2 // Top right
#define ML_DATUM 3 // Middle left
#define CL_DATUM 3 // Centre left, same as above
#define MC_DATUM 4 // Middle centre
#define CC_DATUM 4 // Centre centre, same as above
#define MR_DATUM 5 // Middle right
#define CR_DATUM 5 // Centre right, same as above
#define BL_DATUM 6 // Bottom left
#define BC_DATUM 7 // Bottom centre
#define BR_DATUM 8 // Bottom right
#define L_BASELINE  9 // Left character baseline (Line the 'A' character would sit on)
#define C_BASELINE 10 // Centre character baseline
#define R_BASELINE 11 // Right character baseline

// Default color definitions

// Declarações:
constexpr uint16_t CFK_NO_COLOR     = process_color(0x0000);
constexpr uint16_t CFK_BLACK        = process_color(0x0000);
constexpr uint16_t CFK_WHITE        = process_color(0xffff);
constexpr uint16_t CFK_IVORY        = process_color(0xfffe);
constexpr uint16_t CFK_YELLOW       = process_color(0xffe0);
constexpr uint16_t CFK_GOLD         = process_color(0xfea0);
constexpr uint16_t CFK_ORANGE       = process_color(0xfd20);
constexpr uint16_t CFK_CORAL        = process_color(0xfbea);
constexpr uint16_t CFK_TOMATO       = process_color(0xfb08);
constexpr uint16_t CFK_RED          = process_color(0xf800);
constexpr uint16_t CFK_MAROON       = process_color(0x8000);
constexpr uint16_t CFK_LAVENDER     = process_color(0xe73f);
constexpr uint16_t CFK_PINK         = process_color(0xfe19);
constexpr uint16_t CFK_DEEPPINK     = process_color(0xf8b2);
constexpr uint16_t CFK_PLUM         = process_color(0xdd1b);
constexpr uint16_t CFK_VIOLET       = process_color(0xec1d);
constexpr uint16_t CFK_FUCHSIA      = process_color(0xf81f);
constexpr uint16_t CFK_PURPLE       = process_color(0x8010);
constexpr uint16_t CFK_INDIGO       = process_color(0x4810);
constexpr uint16_t CFK_LIME         = process_color(0x07e0);
constexpr uint16_t CFK_GREEN        = process_color(0x0400);
constexpr uint16_t CFK_OLIVE        = process_color(0x8400);
constexpr uint16_t CFK_AQUA         = process_color(0x07ff);
constexpr uint16_t CFK_SKYBLUE      = process_color(0x867d);
constexpr uint16_t CFK_TEAL         = process_color(0x0410);
constexpr uint16_t CFK_BLUE         = process_color(0x001f);
constexpr uint16_t CFK_NAVY         = process_color(0x0010);
constexpr uint16_t CFK_SILVER       = process_color(0xc618);
constexpr uint16_t CFK_GRAY         = process_color(0x8410);
constexpr uint16_t CFK_WHEAT        = process_color(0xf6f6);
constexpr uint16_t CFK_TAN          = process_color(0xd5b1);
constexpr uint16_t CFK_CHOCOLATE    = process_color(0xd343);
constexpr uint16_t CFK_SADDLEBROWN  = process_color(0x8a22);

// Defines for color palette
//#define INVERTE_BITS_16(x) ((uint16_t)(~(x)))

constexpr uint16_t CFK_COLOR01   = process_color(0xF800);
constexpr uint16_t CFK_COLOR02   = process_color(0xF980);
constexpr uint16_t CFK_COLOR03   = process_color(0xFB00);
constexpr uint16_t CFK_COLOR04   = process_color(0xFC60);
constexpr uint16_t CFK_COLOR05   = process_color(0xFDE0);
constexpr uint16_t CFK_COLOR06   = process_color(0xFF60);
constexpr uint16_t CFK_COLOR07   = process_color(0xDFE0);
constexpr uint16_t CFK_COLOR08   = process_color(0xAFE0);
constexpr uint16_t CFK_COLOR09   = process_color(0x87E0);
constexpr uint16_t CFK_COLOR10   = process_color(0x57E0);
constexpr uint16_t CFK_COLOR11   = process_color(0x27E0);
constexpr uint16_t CFK_COLOR12   = process_color(0x07E2);
constexpr uint16_t CFK_COLOR13   = process_color(0x07E8);
constexpr uint16_t CFK_COLOR14   = process_color(0x07EE);
constexpr uint16_t CFK_COLOR15   = process_color(0x07F3);
constexpr uint16_t CFK_COLOR16   = process_color(0x07F9);
constexpr uint16_t CFK_COLOR17   = process_color(0x07FF);
constexpr uint16_t CFK_COLOR18   = process_color(0x067F);
constexpr uint16_t CFK_COLOR19   = process_color(0x04FF);
constexpr uint16_t CFK_COLOR20   = process_color(0x039F);
constexpr uint16_t CFK_COLOR21   = process_color(0x021F);
constexpr uint16_t CFK_COLOR22   = process_color(0x009F);
constexpr uint16_t CFK_COLOR23   = process_color(0x201F);
constexpr uint16_t CFK_COLOR24   = process_color(0x501F);
constexpr uint16_t CFK_COLOR25   = process_color(0x801F);
constexpr uint16_t CFK_COLOR26   = process_color(0xA81F);
constexpr uint16_t CFK_COLOR27   = process_color(0xD81F);
constexpr uint16_t CFK_COLOR28   = process_color(0xF81D);
constexpr uint16_t CFK_COLOR29   = process_color(0xF817);
constexpr uint16_t CFK_COLOR30   = process_color(0xF811);
constexpr uint16_t CFK_COLOR31   = process_color(0xF80C);
constexpr uint16_t CFK_COLOR32   = process_color(0xF806);

constexpr uint16_t CFK_GREY1     = process_color(0x1082);
constexpr uint16_t CFK_GREY2     = process_color(0x2104);
constexpr uint16_t CFK_GREY3     = process_color(0x3186);
constexpr uint16_t CFK_GREY4     = process_color(0x4228);
constexpr uint16_t CFK_GREY5     = process_color(0x52AA);
constexpr uint16_t CFK_GREY6     = process_color(0x632C);
constexpr uint16_t CFK_GREY7     = process_color(0x73AE);
constexpr uint16_t CFK_GREY8     = process_color(0x8C51);
constexpr uint16_t CFK_GREY9     = process_color(0x9CD3);
constexpr uint16_t CFK_GREY10    = process_color(0xAD55);
constexpr uint16_t CFK_GREY11    = process_color(0xBDD7);
constexpr uint16_t CFK_GREY12    = process_color(0xCE79);
constexpr uint16_t CFK_GREY13    = process_color(0xDEFB);
constexpr uint16_t CFK_GREY14    = process_color(0xEF7D);

const uint16_t CFK_COLOR004 = process_color(0x7800);   // #7F0000
const uint16_t CFK_COLOR005 = process_color(0x7945);   // #7F2A2A
const uint16_t CFK_COLOR006 = process_color(0x7AAA);   // #7F5555
const uint16_t CFK_COLOR007 = process_color(0xF800);   // #FF0000
const uint16_t CFK_COLOR008 = process_color(0xFAAA);   // #FF5555
const uint16_t CFK_COLOR009 = process_color(0xFD55);   // #FFAAAA
const uint16_t CFK_COLOR010 = process_color(0x79E0);   // #7F3F00
const uint16_t CFK_COLOR011 = process_color(0x7AA5);   // #7F552A
const uint16_t CFK_COLOR012 = process_color(0x7B4A);   // #7F6A55
const uint16_t CFK_COLOR013 = process_color(0xFBE0);   // #FF7F00
const uint16_t CFK_COLOR014 = process_color(0xFD4A);   // #FFAA55
const uint16_t CFK_COLOR015 = process_color(0xFEB5);   // #FFD4AA
const uint16_t CFK_COLOR016 = process_color(0x7BE0);   // #7F7F00
const uint16_t CFK_COLOR017 = process_color(0x7BE5);   // #7F7F2A
const uint16_t CFK_COLOR018 = process_color(0x7BEA);   // #7F7F55
const uint16_t CFK_COLOR019 = process_color(0xFFE0);   // #FFFF00
const uint16_t CFK_COLOR020 = process_color(0xFFEA);   // #FFFF55
const uint16_t CFK_COLOR021 = process_color(0xFFF5);   // #FFFFAA
const uint16_t CFK_COLOR022 = process_color(0x3BE0);   // #3F7F00
const uint16_t CFK_COLOR023 = process_color(0x53E5);   // #557F2A
const uint16_t CFK_COLOR024 = process_color(0x6BEA);   // #6A7F55
const uint16_t CFK_COLOR025 = process_color(0x7FE0);   // #7FFF00
const uint16_t CFK_COLOR026 = process_color(0xAFEA);   // #AAFF55
const uint16_t CFK_COLOR027 = process_color(0xD7F5);   // #D4FFAA
const uint16_t CFK_COLOR028 = process_color(0x03E0);   // #007F00
const uint16_t CFK_COLOR029 = process_color(0x2BE5);   // #2A7F2A
const uint16_t CFK_COLOR030 = process_color(0x53EA);   // #557F55
const uint16_t CFK_COLOR031 = process_color(0x07E0);   // #00FF00
const uint16_t CFK_COLOR032 = process_color(0x57EA);   // #55FF55
const uint16_t CFK_COLOR033 = process_color(0xAFF5);   // #AAFFAA
const uint16_t CFK_COLOR034 = process_color(0x03E7);   // #007F3F
const uint16_t CFK_COLOR035 = process_color(0x2BEA);   // #2A7F55
const uint16_t CFK_COLOR036 = process_color(0x53ED);   // #557F6A
const uint16_t CFK_COLOR037 = process_color(0x07EF);   // #00FF7F
const uint16_t CFK_COLOR038 = process_color(0x57F5);   // #55FFAA
const uint16_t CFK_COLOR039 = process_color(0xAFFA);   // #AAFFD4
const uint16_t CFK_COLOR040 = process_color(0x03EF);   // #007F7F
const uint16_t CFK_COLOR041 = process_color(0x2BEF);   // #2A7F7F
const uint16_t CFK_COLOR042 = process_color(0x53EF);   // #557F7F
const uint16_t CFK_COLOR043 = process_color(0x07FF);   // #00FFFF
const uint16_t CFK_COLOR044 = process_color(0x57FF);   // #55FFFF
const uint16_t CFK_COLOR045 = process_color(0xAFFF);   // #AAFFFF
const uint16_t CFK_COLOR046 = process_color(0x01EF);   // #003F7F
const uint16_t CFK_COLOR047 = process_color(0x2AAF);   // #2A557F
const uint16_t CFK_COLOR048 = process_color(0x534F);   // #556A7F
const uint16_t CFK_COLOR049 = process_color(0x03FF);   // #007FFF
const uint16_t CFK_COLOR050 = process_color(0x555F);   // #55AAFF
const uint16_t CFK_COLOR051 = process_color(0xAEBF);   // #AAD4FF
const uint16_t CFK_COLOR052 = process_color(0x000F);   // #00007F
const uint16_t CFK_COLOR053 = process_color(0x294F);   // #2A2A7F
const uint16_t CFK_COLOR054 = process_color(0x52AF);   // #55557F
const uint16_t CFK_COLOR055 = process_color(0x001F);   // #0000FF
const uint16_t CFK_COLOR056 = process_color(0x52BF);   // #5555FF
const uint16_t CFK_COLOR057 = process_color(0xAD5F);   // #AAAAFF
const uint16_t CFK_COLOR058 = process_color(0x380F);   // #3F007F
const uint16_t CFK_COLOR059 = process_color(0x514F);   // #552A7F
const uint16_t CFK_COLOR060 = process_color(0x6AAF);   // #6A557F
const uint16_t CFK_COLOR061 = process_color(0x781F);   // #7F00FF
const uint16_t CFK_COLOR062 = process_color(0xAABF);   // #AA55FF
const uint16_t CFK_COLOR063 = process_color(0xD55F);   // #D4AAFF
const uint16_t CFK_COLOR064 = process_color(0x780F);   // #7F007F
const uint16_t CFK_COLOR065 = process_color(0x794F);   // #7F2A7F
const uint16_t CFK_COLOR066 = process_color(0x7AAF);   // #7F557F
const uint16_t CFK_COLOR067 = process_color(0xF81F);   // #FF00FF
const uint16_t CFK_COLOR068 = process_color(0xFABF);   // #FF55FF
const uint16_t CFK_COLOR069 = process_color(0xFD5F);   // #FFAAFF
const uint16_t CFK_COLOR070 = process_color(0x7807);   // #7F003F
const uint16_t CFK_COLOR071 = process_color(0x794A);   // #7F2A55
const uint16_t CFK_COLOR072 = process_color(0x7AAD);   // #7F556A
const uint16_t CFK_COLOR073 = process_color(0xF80F);   // #FF007F
const uint16_t CFK_COLOR074 = process_color(0xFAB5);   // #FF55AA
const uint16_t CFK_COLOR075 = process_color(0xFD5A);   // #FFAAD4

#if defined(USING_GRAPHIC_LIB)

#define FONT_ROBOTO
#define FONT_SEGMENT7
#define FONT_LED
#define FONT_MUSIC
#define FONT_NOKIAN
#define FONT_PIXEL
#define FONT_SCORED
#define FONT_SCREENMATRIX
#define FONT_URBAN
#define FONT_VACATION



#endif

#endif
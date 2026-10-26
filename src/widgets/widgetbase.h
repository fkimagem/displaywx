//#ifndef WIDGETBASE
//#define WIDGETBASE
#pragma once

#include <Arduino.h>
#include <esp32-hal.h>
#include <stdint.h>

#include "../user_setup.h"
#include "../extras/baseTypes.h"
#include "../extras/wutils.h"
#include "widgetsetup.h"

#include "../extras/color.h"

#if defined(DISP_DEFAULT)
#include <Arduino_GFX_Library.h>
#elif defined(DISP_PCD8544)
#include <Adafruit_GFX.h>
#include <Adafruit_PCD8544.h>
#elif defined(DISP_SSD1306)
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#elif defined(DISP_U8G2)
#include <Arduino.h>
#include <U8g2lib.h>
#else
#error "Unsupported display type"
#endif

#if defined(FONT_ROBOTO)
#include "../fonts/RobotoRegular/RobotoRegular2pt7b.h"
#include "../fonts/RobotoRegular/RobotoRegular3pt7b.h"
#include "../fonts/RobotoRegular/RobotoRegular4pt7b.h"
#include "../fonts/RobotoRegular/RobotoRegular5pt7b.h"
#include "../fonts/RobotoRegular/RobotoRegular6pt7b.h"
#include "../fonts/RobotoRegular/RobotoRegular7pt7b.h"
#include "../fonts/RobotoRegular/RobotoRegular8pt7b.h"
#include "../fonts/RobotoRegular/RobotoRegular9pt7b.h"
#include "../fonts/RobotoRegular/RobotoRegular10pt7b.h"
#include "../fonts/RobotoRegular/RobotoRegular11pt7b.h"
#include "../fonts/RobotoRegular/RobotoRegular12pt7b.h"
#include "../fonts/RobotoRegular/RobotoRegular13pt7b.h"
#include "../fonts/RobotoRegular/RobotoRegular14pt7b.h"
#include "../fonts/RobotoRegular/RobotoRegular15pt7b.h"
#include "../fonts/RobotoRegular/RobotoRegular20pt7b.h"
#include "../fonts/RobotoRegular/RobotoRegular25pt7b.h"
#include "../fonts/RobotoRegular/RobotoRegular30pt7b.h"
#include "../fonts/RobotoRegular/RobotoRegular40pt7b.h"
#include "../fonts/RobotoRegular/RobotoRegular50pt7b.h"

#include "../fonts/RobotoBold/RobotoBold2pt7b.h"
#include "../fonts/RobotoBold/RobotoBold3pt7b.h"
#include "../fonts/RobotoBold/RobotoBold4pt7b.h"
#include "../fonts/RobotoBold/RobotoBold5pt7b.h"
#include "../fonts/RobotoBold/RobotoBold6pt7b.h"
#include "../fonts/RobotoBold/RobotoBold7pt7b.h"
#include "../fonts/RobotoBold/RobotoBold8pt7b.h"
#include "../fonts/RobotoBold/RobotoBold9pt7b.h"
#include "../fonts/RobotoBold/RobotoBold10pt7b.h"
#include "../fonts/RobotoBold/RobotoBold11pt7b.h"
#include "../fonts/RobotoBold/RobotoBold12pt7b.h"
#include "../fonts/RobotoBold/RobotoBold13pt7b.h"
#include "../fonts/RobotoBold/RobotoBold14pt7b.h"
#include "../fonts/RobotoBold/RobotoBold15pt7b.h"
#include "../fonts/RobotoBold/RobotoBold20pt7b.h"
#include "../fonts/RobotoBold/RobotoBold25pt7b.h"
#include "../fonts/RobotoBold/RobotoBold30pt7b.h"
#include "../fonts/RobotoBold/RobotoBold40pt7b.h"
#include "../fonts/RobotoBold/RobotoBold50pt7b.h"

#endif

#if defined(FONT_SEGMENT7)
#include "../fonts/Segment/G7_Segment7_S52pt7b.h"
#include "../fonts/Segment/G7_Segment7_S53pt7b.h"
#include "../fonts/Segment/G7_Segment7_S54pt7b.h"
#include "../fonts/Segment/G7_Segment7_S55pt7b.h"
#include "../fonts/Segment/G7_Segment7_S56pt7b.h"
#include "../fonts/Segment/G7_Segment7_S57pt7b.h"
#include "../fonts/Segment/G7_Segment7_S58pt7b.h"
#include "../fonts/Segment/G7_Segment7_S59pt7b.h"
#include "../fonts/Segment/G7_Segment7_S510pt7b.h"
#include "../fonts/Segment/G7_Segment7_S511pt7b.h"
#include "../fonts/Segment/G7_Segment7_S512pt7b.h"
#include "../fonts/Segment/G7_Segment7_S513pt7b.h"
#include "../fonts/Segment/G7_Segment7_S514pt7b.h"
#include "../fonts/Segment/G7_Segment7_S515pt7b.h"
#include "../fonts/Segment/G7_Segment7_S520pt7b.h"
#include "../fonts/Segment/G7_Segment7_S525pt7b.h"
#include "../fonts/Segment/G7_Segment7_S530pt7b.h"
#include "../fonts/Segment/G7_Segment7_S540pt7b.h"
#include "../fonts/Segment/G7_Segment7_S550pt7b.h"
#endif

#if defined(FONT_LED)
#include "../fonts/Led/LedReal2pt7b.h"
#include "../fonts/Led/LedReal3pt7b.h"
#include "../fonts/Led/LedReal4pt7b.h"
#include "../fonts/Led/LedReal5pt7b.h"
#include "../fonts/Led/LedReal6pt7b.h"
#include "../fonts/Led/LedReal7pt7b.h"
#include "../fonts/Led/LedReal8pt7b.h"
#include "../fonts/Led/LedReal9pt7b.h"
#include "../fonts/Led/LedReal10pt7b.h"
#include "../fonts/Led/LedReal11pt7b.h"
#include "../fonts/Led/LedReal12pt7b.h"
#include "../fonts/Led/LedReal13pt7b.h"
#include "../fonts/Led/LedReal14pt7b.h"
#include "../fonts/Led/LedReal15pt7b.h"
#include "../fonts/Led/LedReal20pt7b.h"
#include "../fonts/Led/LedReal25pt7b.h"
#include "../fonts/Led/LedReal30pt7b.h"
#include "../fonts/Led/LedReal40pt7b.h"
#include "../fonts/Led/LedReal50pt7b.h"
#endif

#if defined(FONT_MUSIC)
#include "../fonts/MusicNet/MUSICNET2pt7b.h"
#include "../fonts/MusicNet/MUSICNET3pt7b.h"
#include "../fonts/MusicNet/MUSICNET4pt7b.h"
#include "../fonts/MusicNet/MUSICNET5pt7b.h"
#include "../fonts/MusicNet/MUSICNET6pt7b.h"
#include "../fonts/MusicNet/MUSICNET7pt7b.h"
#include "../fonts/MusicNet/MUSICNET8pt7b.h"
#include "../fonts/MusicNet/MUSICNET9pt7b.h"
#include "../fonts/MusicNet/MUSICNET10pt7b.h"
#include "../fonts/MusicNet/MUSICNET11pt7b.h"
#include "../fonts/MusicNet/MUSICNET12pt7b.h"
#include "../fonts/MusicNet/MUSICNET13pt7b.h"
#include "../fonts/MusicNet/MUSICNET14pt7b.h"
#include "../fonts/MusicNet/MUSICNET15pt7b.h"
#include "../fonts/MusicNet/MUSICNET20pt7b.h"
#include "../fonts/MusicNet/MUSICNET25pt7b.h"
#include "../fonts/MusicNet/MUSICNET30pt7b.h"
#include "../fonts/MusicNet/MUSICNET40pt7b.h"
#include "../fonts/MusicNet/MUSICNET50pt7b.h"

#endif

#if defined(FONT_NOKIAN)
#include "../fonts/Nokian/Nokian2pt7b.h"
#include "../fonts/Nokian/Nokian3pt7b.h"
#include "../fonts/Nokian/Nokian4pt7b.h"
#include "../fonts/Nokian/Nokian5pt7b.h"
#include "../fonts/Nokian/Nokian6pt7b.h"
#include "../fonts/Nokian/Nokian7pt7b.h"
#include "../fonts/Nokian/Nokian8pt7b.h"
#include "../fonts/Nokian/Nokian9pt7b.h"
#include "../fonts/Nokian/Nokian10pt7b.h"
#include "../fonts/Nokian/Nokian11pt7b.h"
#include "../fonts/Nokian/Nokian12pt7b.h"
#include "../fonts/Nokian/Nokian13pt7b.h"
#include "../fonts/Nokian/Nokian14pt7b.h"
#include "../fonts/Nokian/Nokian15pt7b.h"
#include "../fonts/Nokian/Nokian20pt7b.h"
#include "../fonts/Nokian/Nokian25pt7b.h"
#include "../fonts/Nokian/Nokian30pt7b.h"
#include "../fonts/Nokian/Nokian40pt7b.h"
#include "../fonts/Nokian/Nokian50pt7b.h"
#endif

/*
#define FONT_PIXEL
#define FONT_SCORED
#define FONT_SCREENMATRIX
#define FONT_URBAN
#define FONT_VACATION

*/

#if defined(FONT_PIXEL)
#include "../fonts/Pixel/Pixel3D2pt7b.h"
#include "../fonts/Pixel/Pixel3D3pt7b.h"
#include "../fonts/Pixel/Pixel3D4pt7b.h"
#include "../fonts/Pixel/Pixel3D5pt7b.h"
#include "../fonts/Pixel/Pixel3D6pt7b.h"
#include "../fonts/Pixel/Pixel3D7pt7b.h"
#include "../fonts/Pixel/Pixel3D8pt7b.h"
#include "../fonts/Pixel/Pixel3D9pt7b.h"
#include "../fonts/Pixel/Pixel3D10pt7b.h"
#include "../fonts/Pixel/Pixel3D11pt7b.h"
#include "../fonts/Pixel/Pixel3D12pt7b.h"
#include "../fonts/Pixel/Pixel3D13pt7b.h"
#include "../fonts/Pixel/Pixel3D14pt7b.h"
#include "../fonts/Pixel/Pixel3D15pt7b.h"
#include "../fonts/Pixel/Pixel3D20pt7b.h"
#include "../fonts/Pixel/Pixel3D25pt7b.h"
#include "../fonts/Pixel/Pixel3D30pt7b.h"
#include "../fonts/Pixel/Pixel3D40pt7b.h"
#include "../fonts/Pixel/Pixel3D50pt7b.h"
#endif

#if defined(FONT_SCORED)
#include "../fonts/Scored/Scoreboard2pt7b.h"
#include "../fonts/Scored/Scoreboard3pt7b.h"
#include "../fonts/Scored/Scoreboard4pt7b.h"
#include "../fonts/Scored/Scoreboard5pt7b.h"
#include "../fonts/Scored/Scoreboard6pt7b.h"
#include "../fonts/Scored/Scoreboard7pt7b.h"
#include "../fonts/Scored/Scoreboard8pt7b.h"
#include "../fonts/Scored/Scoreboard9pt7b.h"
#include "../fonts/Scored/Scoreboard10pt7b.h"
#include "../fonts/Scored/Scoreboard11pt7b.h"
#include "../fonts/Scored/Scoreboard12pt7b.h"
#include "../fonts/Scored/Scoreboard13pt7b.h"
#include "../fonts/Scored/Scoreboard14pt7b.h"
#include "../fonts/Scored/Scoreboard15pt7b.h"
#include "../fonts/Scored/Scoreboard20pt7b.h"
#include "../fonts/Scored/Scoreboard25pt7b.h"
#include "../fonts/Scored/Scoreboard30pt7b.h"
#include "../fonts/Scored/Scoreboard40pt7b.h"
#include "../fonts/Scored/Scoreboard50pt7b.h"

#endif

#if defined(FONT_SCREENMATRIX)
#include "../fonts/ScreenMatrix/ScreenMatrix2pt7b.h"
#include "../fonts/ScreenMatrix/ScreenMatrix3pt7b.h"
#include "../fonts/ScreenMatrix/ScreenMatrix4pt7b.h"
#include "../fonts/ScreenMatrix/ScreenMatrix5pt7b.h"
#include "../fonts/ScreenMatrix/ScreenMatrix6pt7b.h"
#include "../fonts/ScreenMatrix/ScreenMatrix7pt7b.h"
#include "../fonts/ScreenMatrix/ScreenMatrix8pt7b.h"
#include "../fonts/ScreenMatrix/ScreenMatrix9pt7b.h"
#include "../fonts/ScreenMatrix/ScreenMatrix10pt7b.h"
#include "../fonts/ScreenMatrix/ScreenMatrix11pt7b.h"
#include "../fonts/ScreenMatrix/ScreenMatrix12pt7b.h"
#include "../fonts/ScreenMatrix/ScreenMatrix13pt7b.h"
#include "../fonts/ScreenMatrix/ScreenMatrix14pt7b.h"
#include "../fonts/ScreenMatrix/ScreenMatrix15pt7b.h"
#include "../fonts/ScreenMatrix/ScreenMatrix20pt7b.h"
#include "../fonts/ScreenMatrix/ScreenMatrix25pt7b.h"
#include "../fonts/ScreenMatrix/ScreenMatrix30pt7b.h"
#include "../fonts/ScreenMatrix/ScreenMatrix40pt7b.h"
#include "../fonts/ScreenMatrix/ScreenMatrix50pt7b.h"
#endif

#if defined(FONT_URBAN)
#include "../fonts/Urban/Urban2pt7b.h"
#include "../fonts/Urban/Urban3pt7b.h"
#include "../fonts/Urban/Urban4pt7b.h"
#include "../fonts/Urban/Urban5pt7b.h"
#include "../fonts/Urban/Urban6pt7b.h"
#include "../fonts/Urban/Urban7pt7b.h"
#include "../fonts/Urban/Urban8pt7b.h"
#include "../fonts/Urban/Urban9pt7b.h"
#include "../fonts/Urban/Urban10pt7b.h"
#include "../fonts/Urban/Urban11pt7b.h"
#include "../fonts/Urban/Urban12pt7b.h"
#include "../fonts/Urban/Urban13pt7b.h"
#include "../fonts/Urban/Urban14pt7b.h"
#include "../fonts/Urban/Urban15pt7b.h"
#include "../fonts/Urban/Urban20pt7b.h"
#include "../fonts/Urban/Urban25pt7b.h"
#include "../fonts/Urban/Urban30pt7b.h"
#include "../fonts/Urban/Urban40pt7b.h"
#include "../fonts/Urban/Urban50pt7b.h"
#endif

#if defined(FONT_VACATION)
#include "../fonts/Vacation/Vacation2pt7b.h"
#include "../fonts/Vacation/Vacation3pt7b.h"
#include "../fonts/Vacation/Vacation4pt7b.h"
#include "../fonts/Vacation/Vacation5pt7b.h"
#include "../fonts/Vacation/Vacation6pt7b.h"
#include "../fonts/Vacation/Vacation7pt7b.h"
#include "../fonts/Vacation/Vacation8pt7b.h"
#include "../fonts/Vacation/Vacation9pt7b.h"
#include "../fonts/Vacation/Vacation10pt7b.h"
#include "../fonts/Vacation/Vacation11pt7b.h"
#include "../fonts/Vacation/Vacation12pt7b.h"
#include "../fonts/Vacation/Vacation13pt7b.h"
#include "../fonts/Vacation/Vacation14pt7b.h"
#include "../fonts/Vacation/Vacation15pt7b.h"
#include "../fonts/Vacation/Vacation20pt7b.h"
#include "../fonts/Vacation/Vacation25pt7b.h"
#include "../fonts/Vacation/Vacation30pt7b.h"
#include "../fonts/Vacation/Vacation40pt7b.h"
#include "../fonts/Vacation/Vacation50pt7b.h"

#endif



#if defined(DFK_SD)
#include "SD.h"
#endif

#define HELPERS

#define LOG_CHECK_ERRORS 0

// START SECTION - MACROS FOR VALIDATION

#if (LOG_CHECK_ERRORS)

#define CHECK_INITIALIZED_BOOL {if(!m_initialized){ESP_LOGW(TAG, "Widget is not initialized"); return false;}}
#define CHECK_INITIALIZED_VOID {if(!m_initialized){ESP_LOGW(TAG, "Widget is not initialized"); return;}}

#define CHECK_LOADED_BOOL {if(!m_loaded){ESP_LOGW(TAG, "Widget is not loaded"); return false;}}
#define CHECK_LOADED_VOID {if(!m_loaded){ESP_LOGW(TAG, "Widget is not loaded"); return;}}

#define CHECK_TFT_BOOL {if(!WidgetBase::objTFT){ESP_LOGW(TAG, "Display object is null"); return false;}}
#define CHECK_TFT_VOID {if(!WidgetBase::objTFT){ESP_LOGW(TAG, "Display object is null"); return;}}

#define CHECK_VISIBLE_VOID {if(!m_visible){ESP_LOGW(TAG, "Widget is not visible"); return;}}
#define CHECK_VISIBLE_BOOL {if(!m_visible){ESP_LOGW(TAG, "Widget is not visible"); return false;}}

#define CHECK_DEBOUNCE_CLICK_BOOL {if(millis() - m_myTime < TIMEOUT_CLICK){ESP_LOGW(TAG, "Debounce click timeout"); return false;}}
#define CHECK_DEBOUNCE_CLICK_VOID {if(millis() - m_myTime < TIMEOUT_CLICK){ESP_LOGW(TAG, "Debounce click timeout"); return;}}

#define CHECK_DEBOUNCE_REDRAW_BOOL {if(millis() - m_myTime < TIMEOUT_REDRAW){ESP_LOGW(TAG, "Debounce redraw timeout"); return false;}}
#define CHECK_DEBOUNCE_REDRAW_VOID {if(millis() - m_myTime < TIMEOUT_REDRAW){ESP_LOGW(TAG, "Debounce redraw timeout"); return;}}

#define CHECK_DEBOUNCE_FAST_REDRAW_BOOL {if(millis() - m_myTime < TIMEOUT_FAST_REDRAW){ESP_LOGW(TAG, "Debounce fast redraw timeout"); return false;}}
#define CHECK_DEBOUNCE_FAST_REDRAW_VOID {if(millis() - m_myTime < TIMEOUT_FAST_REDRAW){ESP_LOGW(TAG, "Debounce fast redraw timeout"); return;}}

#define CHECK_ENABLED_BOOL {if(!m_enabled){ESP_LOGW(TAG, "Widget is disabled"); return false;}}
#define CHECK_ENABLED_VOID {if(!m_enabled){ESP_LOGW(TAG, "Widget is disabled"); return;}}

#define CHECK_LOCKED_BOOL {if(m_locked){ESP_LOGW(TAG, "Widget is locked"); return false;}}
#define CHECK_LOCKED_VOID {if(m_locked){ESP_LOGW(TAG, "Widget is locked"); return;}}

#define CHECK_USINGKEYBOARD_BOOL {if(WidgetBase::usingKeyboard){ESP_LOGW(TAG, "Using keyboard"); return false;}}
#define CHECK_USINGKEYBOARD_VOID {if(WidgetBase::usingKeyboard){ESP_LOGW(TAG, "Using keyboard"); return;}}

#define CHECK_CURRENTSCREEN_BOOL {if(WidgetBase::currentScreen != m_screen){ESP_LOGW(TAG, "Not on current screen"); return false;}}
#define CHECK_CURRENTSCREEN_VOID {if(WidgetBase::currentScreen != m_screen){ESP_LOGW(TAG, "Not on current screen"); return;}}

#define CHECK_SHOULDREDRAW_BOOL {if(!m_shouldRedraw){ESP_LOGW(TAG, "Should not redraw"); return false;}}
#define CHECK_SHOULDREDRAW_VOID {if(!m_shouldRedraw){ESP_LOGW(TAG, "Should not redraw"); return;}}

#define CHECK_POINTER_TOUCH_NULL_BOOL {if(_xTouch == nullptr || _yTouch == nullptr){ ESP_LOGW(TAG, "Touch coordinates are null"); return false;}}
#define CHECK_POINTER_TOUCH_NULL_VOID {if(_xTouch == nullptr || _yTouch == nullptr){ ESP_LOGW(TAG, "Touch coordinates are null"); return;}}

#else

#define CHECK_INITIALIZED_BOOL {if(!m_initialized){ return false;}}
#define CHECK_INITIALIZED_VOID {if(!m_initialized){ return;}}

#define CHECK_LOADED_BOOL {if(!m_loaded){ return false;}}
#define CHECK_LOADED_VOID {if(!m_loaded){return;}}

#define CHECK_TFT_BOOL {if(!WidgetBase::objTFT){ return false;}}
#define CHECK_TFT_VOID {if(!WidgetBase::objTFT){return;}}

#define CHECK_VISIBLE_VOID {if(!m_visible){return;}}
#define CHECK_VISIBLE_BOOL {if(!m_visible){return false;}}

#define CHECK_DEBOUNCE_CLICK_BOOL {if(millis() - m_myTime < TIMEOUT_CLICK){return false;}}
#define CHECK_DEBOUNCE_CLICK_VOID {if(millis() - m_myTime < TIMEOUT_CLICK){return;}}

#define CHECK_DEBOUNCE_LONG_CLICK_BOOL {if(millis() - m_myTime < TIMEOUT_CLICK_LONG){return false;}}
#define CHECK_DEBOUNCE_LONG_CLICK_VOID {if(millis() - m_myTime < TIMEOUT_CLICK_LONG){return;}}

#define CHECK_DEBOUNCE_REDRAW_BOOL {if(millis() - m_myTime < TIMEOUT_REDRAW){return false;}}
#define CHECK_DEBOUNCE_REDRAW_VOID {if(millis() - m_myTime < TIMEOUT_REDRAW){return;}}

#define CHECK_DEBOUNCE_FAST_REDRAW_BOOL {if(millis() - m_myTime < TIMEOUT_FAST_REDRAW){return false;}}
#define CHECK_DEBOUNCE_FAST_REDRAW_VOID {if(millis() - m_myTime < TIMEOUT_FAST_REDRAW){return;}}


#define CHECK_ENABLED_BOOL {if(!m_enabled){return false;}}
#define CHECK_ENABLED_VOID {if(!m_enabled){return;}}

#define CHECK_LOCKED_BOOL {if(m_locked){return false;}}
#define CHECK_LOCKED_VOID {if(m_locked){return;}}

#define CHECK_USINGKEYBOARD_BOOL {if(WidgetBase::usingKeyboard){return false;}}
#define CHECK_USINGKEYBOARD_VOID {if(WidgetBase::usingKeyboard){return;}}

#define CHECK_CURRENTSCREEN_BOOL {if(WidgetBase::currentScreen != m_screen){return false;}}
#define CHECK_CURRENTSCREEN_VOID {if(WidgetBase::currentScreen != m_screen){return;}}

#define CHECK_SHOULDREDRAW_BOOL {if(!m_shouldRedraw){return false;}}
#define CHECK_SHOULDREDRAW_VOID {if(!m_shouldRedraw){return;}}

#define CHECK_POINTER_TOUCH_NULL_BOOL {if(_xTouch == nullptr || _yTouch == nullptr){ return false;}}
#define CHECK_POINTER_TOUCH_NULL_VOID {if(_xTouch == nullptr || _yTouch == nullptr){ return;}}



#endif

// END SECTION - MACROS FOR VALIDATION


/// @brief Classe base abstrata para todos os widgets.
/// @details Esta classe fornece funcionalidade comum para todos os widgets no sistema DisplayWX.
///          Define métodos virtuais puros que devem ser implementados pelas classes derivadas,
///          além de fornecer funcionalidades estáticas compartilhadas como gerenciamento de cores,
///          callbacks, fontes e referências ao display. Esta classe não pode ser instanciada diretamente
///          e deve ser herdada por classes de widgets concretos como Button, TextBox, Gauge, etc.
///          Todos os widgets herdam de @ref WidgetBase e implementam seus métodos virtuais puros.
class WidgetBase
{
private:
  static const char* TAG; ///< Tag estática para identificação em logs.
  static uint16_t convertToRGB565(uint8_t r, uint8_t g, uint8_t b);
  static void extract565toRGB(uint16_t color565, uint8_t &r, uint8_t &g, uint8_t &b);

public:
  enum class CallbackOrigin{
    TOUCH = 1,  ///< Callback originado de um toque no widget.
    SELF = 2    ///< Callback originado de uma ação interna do widget.
  };

  static QueueHandle_t xFilaCallback; ///< Fila de callbacks para processamento assíncrono.
  static bool usingKeyboard;    ///< Indica se um teclado virtual está em uso.
  static uint8_t currentScreen; ///< Número da tela atual.
  static bool showingLog;       ///< Indica se o log está sendo exibido.
  #if defined(USING_GRAPHIC_LIB)
  static const GFXfont *fontNormal; ///< Ponteiro para a fonte normal.
  static const GFXfont *fontBold;   ///< Ponteiro para a fonte em negrito.
  #endif
  static void addCallback(functionCB_t callback, CallbackOrigin origin);
  static uint16_t screenWidth;             ///< Largura da tela.
  static uint16_t screenHeight;            ///< Altura da tela.
  static bool lightMode;                   ///< True para modo claro, False para modo escuro.
  static functionLoadScreen_t loadScreen; ///< Ponteiro para a função que carrega a tela.
  static uint16_t backgroundColor;         ///< Cor de fundo para os widgets.
  

  //static uint16_t lightenColor565(unsigned short color, float factor);
  //static uint16_t lightenToWhite565(uint16_t color, float factor);
  //static uint16_t darkenColor565(unsigned short color, float factor);
  //static uint16_t blendColors(uint16_t color1, uint16_t color2, float factor);

#if defined(DISP_DEFAULT)
  static Arduino_GFX *objTFT; ///< Ponteiro para o objeto de display Arduino GFX.
#elif defined(DISP_PCD8544)
  static Adafruit_PCD8544 *objTFT; ///< Ponteiro para o objeto de display PCD8544.
#elif defined(DISP_SSD1306)
  static Adafruit_SSD1306 *objTFT; ///< Ponteiro para o objeto de display SSD1306.
#elif defined(DISP_U8G2)
  static U8G2 *objTFT; ///< Ponteiro para o objeto de display U8G2.
#else
#error "Escolha um modelo de display em user_setup.h"
#endif

#if defined(DFK_SD)
  static fs::SDFS *mySD; ///< Ponteiro para o sistema de arquivos SD.
#endif

  WidgetBase(uint16_t _x, uint16_t _y, uint8_t _screen);

  virtual ~WidgetBase();
  virtual bool detectTouch(uint16_t *_xTouch, uint16_t *_yTouch) = 0;
  virtual functionCB_t getCallbackFunc() = 0;
  virtual void show() = 0;
  virtual void hide() = 0;
  virtual void forceUpdate() = 0;
  virtual void redraw() = 0;
  virtual void onRelease();  ///< Called when touch is released or leaves widget area
  
  bool showingMyScreen();
  
  // State management methods
  bool isInitialized() const;
  bool isEnabled() const;
  void setEnabled(bool enabled);
  bool isValidState() const;
  bool validateParameters() const;
  
  // Lock management methods
  void lock();
  void unlock();
  bool isLocked() const;
  
#if defined(USING_GRAPHIC_LIB)
  static void recalculateTextPosition(const char* _texto, uint16_t *_x, uint16_t *_y, uint8_t _datum);
  static void setFontNull() {if (WidgetBase::objTFT) WidgetBase::objTFT->setFont((GFXfont *)0);} ///< Sets the font to null.
  #endif

protected:
  bool m_visible;        ///< True se o widget está visível.
  int16_t m_xPos;              ///< Posição X do widget.
  int16_t m_yPos;              ///< Posição Y do widget.
  uint8_t m_screen;             ///< Índice da tela para o widget.
  bool m_loaded;        ///< True se o widget foi inicializado.
  bool m_enabled;        ///< True se o widget está habilitado.
  bool m_initialized;   ///< True se o widget está propriamente inicializado.
  bool m_shouldRedraw;   ///< Flag indicando se o widget deve ser redesenhado.
  bool m_isPressed;          ///< True when widget is currently being touched/pressed
  bool m_locked;        ///< True se o widget está bloqueado (previne interação).
  unsigned long m_myTime;   ///< Timestamp para manipulação de funções relacionadas a tempo (debounce, etc).
  functionCB_t m_callback; ///< Função callback para executar quando o widget é clicado.

#if defined(USING_GRAPHIC_LIB)
  const GFXfont* getBestRobotoBold(uint16_t availableWidth, uint16_t availableHeight, const char* texto);
  const GFXfont* getBestFontForArea(const char* text,uint16_t width,uint16_t height,const GFXfont* const fonts[],size_t fontCount);
  int getFirstLettersForSpace(const char* textoCompleto, uint16_t width, uint16_t height, char* out, size_t outSize);
  int getLastLettersForSpace(const char* textoCompleto, uint16_t width, uint16_t height, char* out, size_t outSize);
  
#endif

  void updateFont(FontType _f);

  /**
   * @brief Marks widget as pressed
   * @param pressed True if widget is being pressed, false otherwise
   * @details Called internally when touch is detected. Sets m_isPressed to true.
   */
  void setPressed(bool pressed);

#if defined(USING_GRAPHIC_LIB)
  void printText(const char* _texto, uint16_t _x, uint16_t _y, uint8_t _datum, TextBound_t &lastTextBoud, uint16_t _colorPadding);
  void printText(const char* _texto, uint16_t _x, uint16_t _y, uint8_t _datum);
  TextBound_t getTextBounds(const char* str, int16_t x, int16_t y);
  void drawRotatedImageOptimized(uint16_t *image, int16_t width, int16_t height, float angle, int16_t pivotX, int16_t pivotY, int16_t drawX, int16_t drawY);
#endif

  void showOrigin(uint16_t color);
};

//#endif
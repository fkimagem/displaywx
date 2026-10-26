/**
 * @mainpage DisplayFK Library Documentation
 *
 * @brief A C++ library for creating graphical user interfaces on microcontrollers
 * based on the Arduino_GFX library.
 *
 * @details
 * This library provides a comprehensive framework for developing graphical user interfaces
 * on microcontrollers. It offers a wide range of widgets and controls that can be easily
 * integrated into your projects.
 *
 * ## Features
 * - Touch screen support
 * - Multiple widget types (buttons, sliders, gauges, etc.)
 * - SD card integration
 * - Logging system
 * - Event handling
 * - Screen management
 *
 * ## Getting Started
 * To use this library in your project:
 * 1. Include the necessary header files
 * 2. Initialize the DisplayFK object
 * 3. Configure your widgets
 * 4. Start the main loop
 *
 * ## Widget Types
 * The library supports various widget types:
 * - Buttons (Text, Circle, Rectangle)
 * - Sliders (Horizontal, Vertical)
 * - Gauges and Charts
 * - Text Input (TextBox, NumberBox)
 * - Indicators (LED, Label)
 * - And more...
 *
 * @note This library requires Arduino_GFX as a dependency
 * @see Arduino_GFX documentation for display driver requirements
 *
 * @version 1.0.0
 * @author Your Name
 * @copyright Copyright (c) 2024
 */

#ifndef DISPLAYFK_H
#define DISPLAYFK_H

// #define DEBUG_TOUCH
#define DEBUG_DISPLAY

#include "widgets/widgetsetup.h"
#include "../user_setup.h"
#include "check_touch.h"
#include "check_camera.h"

#if defined(CAM_OV02C10)
#include "camera/ov02c10/ov02c10/ov02c10.h"
#endif
#if defined(CAM_OV5647)
#include "camera/ov5647/ov5647/ov5647.h"
#endif
#if defined(CAM_SC2336)
#include "camera/sc2336/sc2336/sc2336.h"
#endif
#if defined(CAM_OV2710)
#include "camera/ov2710/ov2710/ov2710.h"
#endif
#if defined(HAS_CAMERA_MIPI) && defined(DFK_CAMERA_MIPI)
#include "camera/cameracontroller/cameracontroller.h"
#endif

#include <Arduino.h>
#include <atomic>
#if defined(USE_SPIFFS)
#include "SPIFFS.h"
#endif
#if defined(USE_FATFS)
#include "FFat.h"
#endif
#include <FS.h>
#include <Preferences.h>
#include <SPI.h>
#include <memory>

#include "touch/touch.h"


// #include "soc/timer_group_struct.h" //watchdog
// #include "soc/timer_group_reg.h"    //watchdog
#include "freertos/queue.h"
#include "freertos/timers.h"
#if defined(PLATFORMIO)
#include "esp_task_wdt.h"
#elif defined(ARDUINO)
#include <esp_task_wdt.h>
#endif
#include "extras/check_version.h"

#include "widgets/widgetbase.h"

#include "touch_widgets.h"
#include "output_widgets.h"

#include "freertos/queue.h"
#include <freertos/semphr.h>


#if defined(CONFIG_IDF_TARGET_ESP32P4) && defined(DISP_DEFAULT)
static const lcd_init_cmd_t st7701_init_operations[] = {
    // {cmd, { data }, data_size, delay_ms}
    {0xFF, (uint8_t[]){0x77, 0x01, 0x00, 0x00, 0x13}, 5, 0},
    {0xEF, (uint8_t[]){0x08}, 1, 0},
    {0xFF, (uint8_t[]){0x77, 0x01, 0x00, 0x00, 0x10}, 5, 0},
    {0xC0, (uint8_t[]){0x63, 0x00}, 2, 0},
    {0xC1, (uint8_t[]){0x0D, 0x02}, 2, 0},
    {0xC2, (uint8_t[]){0x10, 0x08}, 2, 0},
    {0xCC, (uint8_t[]){0x10}, 1, 0},
    {0xB0, (uint8_t[]){0x80, 0x09, 0x53, 0x0C, 0xD0, 0x07, 0x0C, 0x09, 0x09, 0x28, 0x06, 0xD4, 0x13, 0x69, 0x2B, 0x71}, 16, 0},
    {0xB1, (uint8_t[]){0x80, 0x94, 0x5A, 0x10, 0xD3, 0x06, 0x0A, 0x08, 0x08, 0x25, 0x03, 0xD3, 0x12, 0x66, 0x6A, 0x0D}, 16, 0},
    {0xFF, (uint8_t[]){0x77, 0x01, 0x00, 0x00, 0x11}, 5, 0},
    {0xB0, (uint8_t[]){0x5D}, 1, 0},
    {0xB1, (uint8_t[]){0x58}, 1, 0},
    {0xB2, (uint8_t[]){0x87}, 1, 0},
    {0xB3, (uint8_t[]){0x80}, 1, 0},
    {0xB5, (uint8_t[]){0x4E}, 1, 0},
    {0xB7, (uint8_t[]){0x85}, 1, 0},
    {0xB8, (uint8_t[]){0x21}, 1, 0},
    {0xB9, (uint8_t[]){0x10, 0x1F}, 2, 0},
    {0xBB, (uint8_t[]){0x03}, 1, 0},
    {0xBC, (uint8_t[]){0x00}, 1, 0},
    {0xC1, (uint8_t[]){0x78}, 1, 0},
    {0xC2, (uint8_t[]){0x78}, 1, 0},
    {0xD0, (uint8_t[]){0x88}, 1, 0},
    {0xE0, (uint8_t[]){0x00, 0x3A, 0x02}, 3, 0},
    {0xE1, (uint8_t[]){0x04, 0xA0, 0x00, 0xA0, 0x05, 0xA0, 0x00, 0xA0, 0x00, 0x40, 0x40}, 11, 0},
    {0xE2, (uint8_t[]){0x30, 0x00, 0x40, 0x40, 0x32, 0xA0, 0x00, 0xA0, 0x00, 0xA0, 0x00, 0xA0, 0x00}, 13, 0},
    {0xE3, (uint8_t[]){0x00, 0x00, 0x33, 0x33}, 4, 0},
    {0xE4, (uint8_t[]){0x44, 0x44}, 2, 0},
    {0xE5, (uint8_t[]){0x09, 0x2E, 0xA0, 0xA0, 0x0B, 0x30, 0xA0, 0xA0, 0x05, 0x2A, 0xA0, 0xA0, 0x07, 0x2C, 0xA0, 0xA0}, 16, 0},
    {0xE6, (uint8_t[]){0x00, 0x00, 0x33, 0x33}, 4, 0},
    {0xE7, (uint8_t[]){0x44, 0x44}, 2, 0},
    {0xE8, (uint8_t[]){0x08, 0x2D, 0xA0, 0xA0, 0x0A, 0x2F, 0xA0, 0xA0, 0x04, 0x29, 0xA0, 0xA0, 0x06, 0x2B, 0xA0, 0xA0}, 16, 0},
    {0xEB, (uint8_t[]){0x00, 0x00, 0x4E, 0x4E, 0x00, 0x00, 0x00}, 7, 0},
    {0xEC, (uint8_t[]){0x08, 0x01}, 2, 0},
    {0xED, (uint8_t[]){0xB0, 0x2B, 0x98, 0xA4, 0x56, 0x7F, 0xFF, 0xFF, 0xFF, 0xFF, 0xF7, 0x65, 0x4A, 0x89, 0xB2, 0x0B}, 16, 0},
    {0xEF, (uint8_t[]){0x08, 0x08, 0x08, 0x45, 0x3F, 0x54}, 6, 0},
    {0xFF, (uint8_t[]){0x77, 0x01, 0x00, 0x00, 0x00}, 5, 0},
    {0x11, (uint8_t[]){0x00}, 1, 120},  // Sleep Out - delay de 120ms
    {0x29, (uint8_t[]){0x00}, 1, 20},   // Display On - delay de 20ms
};

static const lcd_init_cmd_t ek79007_init_operations[] = {
    {0xB2, (uint8_t[]){0x10}, 1, 0},
    {0x80, (uint8_t[]){0x8B}, 1, 0},
    {0x81, (uint8_t[]){0x78}, 1, 0},
    {0x82, (uint8_t[]){0x84}, 1, 0},
    {0x83, (uint8_t[]){0x88}, 1, 0},
    {0x84, (uint8_t[]){0xA8}, 1, 0},
    {0x85, (uint8_t[]){0xE3}, 1, 0},
    {0x86, (uint8_t[]){0x88}, 1, 0},
    {0x11, (uint8_t[]){0x00}, 0, 120},
    {0x29, (uint8_t[]){0x00}, 0, 20},
};

#include "displays/ili9881c_waveshare_7in.h"
#endif
/**
 * @brief Maximum length for a single log line
 */
#define MAX_LINE_LENGTH (64)

/**
 * @brief Length of the log queue buffer
 */
#define LOG_LENGTH (10)

/**
 * @brief Maximum size for the log file in bytes
 */
#define LOG_MAX_SIZE (1000)

/**
 * @brief Size of the string pool for temporary strings
 */
#define STRING_POOL_SIZE (256)

/**
 * @brief Number of string buffers in the pool
 */
#define STRING_POOL_COUNT (8)

/**
 * @brief Structure to hold a single log message
 */
typedef struct
{
    char line[MAX_LINE_LENGTH]; ///< The actual log message text
    uint8_t line_length;        ///< Length of the message
} logMessage_t;

/**
 * @brief Structure to track string allocations (C-style)
 */
typedef struct {
    char* ptr;
    bool isFromPool;
    uint32_t timestamp;
    const char* caller;
} StringAllocation_t;

/**
 * @brief Simple string pool for temporary string allocations with C-style tracking
 */
class StringPool
{
private:
    char pool[STRING_POOL_COUNT][STRING_POOL_SIZE];
    bool inUse[STRING_POOL_COUNT];
    StringAllocation_t m_allocations[STRING_POOL_COUNT];
    uint8_t m_activeCount;
    uint32_t m_allocationCount;
    uint32_t m_deallocationCount;

public:
    StringPool() : m_activeCount(0), m_allocationCount(0), m_deallocationCount(0)
    {
        for (int i = 0; i < STRING_POOL_COUNT; i++)
        {
            inUse[i] = false;
            m_allocations[i].ptr = nullptr;
            m_allocations[i].isFromPool = false;
            m_allocations[i].timestamp = 0;
            m_allocations[i].caller = nullptr;
        }
    }

    char* allocate(const char* caller = nullptr)
    {
        for (int i = 0; i < STRING_POOL_COUNT; i++)
        {
            if (!inUse[i])
            {
                inUse[i] = true;
                char* ptr = pool[i];
                
                // Track allocation
                if (m_activeCount < STRING_POOL_COUNT) {
                    m_allocations[m_activeCount].ptr = ptr;
                    m_allocations[m_activeCount].isFromPool = true;
                    m_allocations[m_activeCount].timestamp = millis();
                    m_allocations[m_activeCount].caller = caller;
                    m_activeCount++;
                }
                m_allocationCount++;
                
                return ptr;
            }
        }
        return nullptr; // Pool exhausted
    }

    void deallocate(char* ptr)
    {
        if (ptr)
        {
            for (int i = 0; i < STRING_POOL_COUNT; i++)
            {
                if (pool[i] == ptr)
                {
                    inUse[i] = false;
                    pool[i][0] = '\0'; // Clear the string
                    
                    // Track deallocation
                    m_deallocationCount++;
                    
                    // Remove from tracking
                    for (int j = 0; j < m_activeCount; j++)
                    {
                        if (m_allocations[j].ptr == ptr && m_allocations[j].isFromPool)
                        {
                            // Shift remaining elements left
                            for (int k = j; k < m_activeCount - 1; k++)
                            {
                                m_allocations[k] = m_allocations[k + 1];
                            }
                            m_activeCount--;
                            break;
                        }
                    }
                    return;
                }
            }
        }
    }
    
    /**
     * @brief Get allocation statistics
     */
    void getStats(uint32_t& activeAllocations, uint32_t& totalAllocations, uint32_t& totalDeallocations) const
    {
        activeAllocations = m_activeCount;
        totalAllocations = m_allocationCount;
        totalDeallocations = m_deallocationCount;
    }
    
    /**
     * @brief Get active allocations for debugging
     */
    const StringAllocation_t* getActiveAllocations() const
    {
        return m_allocations;
    }
    
    /**
     * @brief Get count of active allocations
     */
    uint8_t getActiveCount() const
    {
        return m_activeCount;
    }
    
    /**
     * @brief Cleanup all tracked allocations
     */
    void cleanup()
    {
        for (int i = 0; i < m_activeCount; i++)
        {
            if (m_allocations[i].isFromPool)
            {
                deallocate(m_allocations[i].ptr);
            }
        }
        m_activeCount = 0;
    }
};

enum class TouchEventType
{
    NONE = 0,
    TOUCH_DOWN = 1,
    TOUCH_HOLD = 2,
    TOUCH_UP = 3
};

enum class TouchSwipeDirection
{
    NONE = 0,
    UP = 1,
    DOWN = 2,
    LEFT = 3,
    RIGHT = 4
};

// Descomentar a linha abaixo para habilitar o recurso onRelease
// Quando comentado, todo o código de rastreamento de touch é desabilitado
#define ENABLE_ON_RELEASE

/// @brief Represents the main display framework class, managing various widget types, SD card functionality, and touch input.
class DisplayFK
{
public:
    // Variáveis públicas estáticas
    static DisplayFK *instance;
    static bool sdcardOK; ///< Indicates if the SD card is successfully initialized.

    // Variáveis públicas
    std::unique_ptr<TouchScreen> touchExterno; ///< External touch screen object.

    // Métodos públicos estáticos
    static void TaskEventoTouch(void *pvParamenters);
#if defined(HAS_CAMERA_MIPI) && defined(DFK_CAMERA_MIPI)
    static void TaskCamera(void *pvParameters);
#endif

    // Métodos públicos
    DisplayFK();
    ~DisplayFK();

    void setup();
    void startKeyboards();
    void refreshScreen();
    void loadScreen(functionLoadScreen_t screen);

#if defined(DISP_DEFAULT)
    void setDrawObject(Arduino_GFX *objTFT); ///< Pointer to the Arduino display object.
#if defined(CONFIG_IDF_TARGET_ESP32P4)
    void setDrawObject(Arduino_DSI_Display *objTFT);
#endif
#elif defined(DISP_PCD8544)
    void setDrawObject(Adafruit_PCD8544 *objTFT); ///< Pointer to the PCD8544 display object.
#elif defined(DISP_SSD1306)
    void setDrawObject(Adafruit_SSD1306 *objTFT); ///< Pointer to the SSD1306 display object.
#elif defined(DISP_U8G2)
    void setDrawObject(U8G2 *objTFT); ///< Pointer to the U8G2 display object.
#else
#error "Choose a display model on user_setup.h"
#endif

    /// Holds the draw mutex until finishCustomDraw(). Widget callbacks run outside this mutex, so a callback may call startCustomDraw().
    void startCustomDraw();
    /// Releases the draw mutex taken by startCustomDraw().
    void finishCustomDraw();
    void drawWidgetsOnScreen(const uint8_t currentScreenIndex);
#if defined(USING_GRAPHIC_LIB)
    void setFontNormal(const GFXfont *_font);
    void setFontBold(const GFXfont *_font);
    void printText(const char *_texto, uint16_t _x, uint16_t _y, uint8_t _datum, uint16_t _colorText, uint16_t _colorPadding, const GFXfont *_font);
#endif
    void createTask(bool enableWatchdog, uint16_t timeout_s = 3);
    void drawPng(uint16_t _x, uint16_t _y, const uint16_t _colors[], const uint8_t _mask[], uint16_t _w, uint16_t _h);
    bool waveshareDsiAuxBegin(int sda, int scl);
    bool waveshareDsiAuxBacklight(uint8_t brightness);
    void enableTouchLog();
    void disableTouchLog();
    void addLog(const char *data);
    void displayfkTask();
    void cameraTask();
    TaskHandle_t getTaskHandle() const;
#if defined(HAS_CAMERA_MIPI) && defined(DFK_CAMERA_MIPI)
    CameraController &getCamera();
    void setRotationCamera(uint8_t rotation, bool mirrorX = false, bool mirrorY = false);
    bool startCamera(int sccbPort, int8_t sclPin, int8_t sdaPin, uint32_t i2cFreq = 0, int8_t resetPin = -1, int8_t pwdnPin = -1, uint16_t sensorWidth = 0, uint16_t sensorHeight = 0);
    bool isCameraReady() const;
    void setCameraPreviewFps(uint8_t fps);
    uint8_t cameraPreviewFps() const;
    bool beginCameraConfig(CameraMipi *widget);
#endif
    void setupAutoClick(uint32_t intervalMs, uint16_t x, uint16_t y);
    void startAutoClick();
    void stopAutoClick();
    bool isRunningAutoClick() const;
    void blockLoopTask();
    void freeLoopTask();

    // Memory management utilities
    void freeStringFromPool(const char *str);
    void cleanup();  // Manual cleanup function

    // Input validation utilities
    bool validateInput(const char* input, size_t maxLength) const;
    bool validatePinNumber(int8_t pin) const;
    bool validateFrequency(int frequency) const;
    bool validateDimensions(uint16_t width, uint16_t height) const;
    bool validateArray(const void* array, uint8_t count) const;
    bool validateString(const char* str, size_t maxLength) const;
    
    // Initialization utilities
    void initializeWidgetFlags();
    bool validateInitialization() const;
    
    // Cleanup utilities
    bool waitForUiTaskToStop();
    void stopUiTaskAndDelete();
#if defined(HAS_CAMERA_MIPI) && defined(DFK_CAMERA_MIPI)
    void stopCameraTaskAndDelete();
#endif
    void cleanupFreeRTOSResources();
    void cleanupDynamicMemory();
    void cleanupWidgets();
    bool validateCleanup() const;
    
    // StringPool tracking methods
    void printStringPoolStats() const;
    void cleanupStringPool() const;
    
    // Safe methods without exceptions (ESP32 compatible)
    bool startKeyboardsSafe();
    bool createTimerSafe();
    bool createSemaphoreSafe();
    
    char* allocateStringSafe(const char* caller);
    void deallocateStringSafe(char* str);
    bool startSDSafe(uint8_t pinCS, SPIClass *spiShared, int hz);

#if defined(HAS_TOUCH)
#if defined(TOUCH_XPT2046)
    void checkCalibration();
    void recalibrate();
#endif

    void startTouch(uint16_t w, uint16_t h, uint8_t _rotation, SPIClass *_sharedSPI);
    void setTouchCorners(int x0, int x1, int y0, int y1);
    void setInvertAxis(bool invert_x, bool invert_y);
    void setSwapAxis(bool swap);

#if defined(TOUCH_XPT2046)
    void startTouchXPT2046(uint16_t w, uint16_t h, uint8_t _rotation, int8_t pinCS, SPIClass *_sharedSPI, Arduino_GFX *_objTFT, int touchFrequency, int displayFrequency = 0, int displayPinCS = 0);
    void startTouchXPT2046(uint16_t w, uint16_t h, uint8_t _rotation, int8_t pinSclk, int8_t pinMosi, int8_t pinMiso, int8_t pinCS, Arduino_GFX *_objTFT, int touchFrequency, int displayFrequency = 0, int displayPinCS = -1);
#elif defined(TOUCH_FT6236U)
    void startTouchFT6236U(uint16_t w, uint16_t h, uint8_t _rotation, int8_t pinSDA, int8_t pinSCL, uint8_t pinINT, int8_t pinRST);
#elif defined(TOUCH_FT6336)
    void startTouchFT6336(uint16_t w, uint16_t h, uint8_t _rotation, int8_t pinSDA, int8_t pinSCL, int8_t pinINT, int8_t pinRST);
#elif defined(TOUCH_CST816)
    void startTouchCST816(uint16_t w, uint16_t h, uint8_t _rotation, uint8_t pinSDA, int8_t pinSCL, int8_t pinINT, int8_t pinRST);
#elif defined(TOUCH_GT911)
    void startTouchGT911(uint16_t w, uint16_t h, uint8_t _rotation, int8_t pinSDA, int8_t pinSCL, int8_t pinINT, int8_t pinRST);
#elif defined(TOUCH_GT911_WAVESHARE)
    void startTouchGT911Waveshare(uint16_t w, uint16_t h, uint8_t _rotation, int8_t pinSDA, int8_t pinSCL);
#elif defined(TOUCH_GSL3680)
    void startTouchGSL3680(uint16_t w, uint16_t h, uint8_t _rotation, int8_t pinSDA, int8_t pinSCL, int8_t pinINT, int8_t pinRST);
#elif defined(TOUCH_CST820)
    void startTouchCST820(uint16_t w, uint16_t h, uint8_t _rotation, int8_t pinSDA, int8_t pinSCL, int8_t pinINT, int8_t pinRST);

#endif
#endif

    void changeWTD();

#ifdef DFK_CHECKBOX
    bool setCheckboxSafe(CheckBox *array[], uint8_t amount);
    void setCheckbox(CheckBox *array[], uint8_t amount);
#endif

#ifdef DFK_CIRCLEBTN
    void setCircleButton(CircleButton *array[], uint8_t amount);
#endif

#ifdef DFK_GAUGE
    void setGauge(GaugeSuper *array[], uint8_t amount);
#endif

#ifdef DFK_CIRCULARBAR
    void setCircularBar(CircularBar *array[], uint8_t amount);
#endif

#ifdef DFK_HSLIDER
    void setHSlider(HSlider *array[], uint8_t amount);
#endif

#ifdef DFK_LABEL
    void setLabel(Label *array[], uint8_t amount);
#endif

#ifdef DFK_LED
    void setLed(Led *array[], uint8_t amount);
#endif

#ifdef DFK_LINECHART
    void setLineChart(LineChart *array[], uint8_t amount);
#endif

#ifdef DFK_NUMBERBOX
    void setNumberbox(NumberBox *array[], uint8_t amount);
    void setupNumpad();
    void insertCharNumpad(char c);
#endif

#ifdef DFK_RADIO
    void setRadioGroup(RadioGroup *array[], uint8_t amount);
#endif

#ifdef DFK_RECTBTN
    void setRectButton(RectButton *array[], uint8_t amount);
#endif

#ifdef DFK_TEXTBOX
    void setTextbox(TextBox *array[], uint8_t amount);
    void setupKeyboard();
    void insertCharTextbox(char c);
#endif

#ifdef DFK_TOGGLE
    void setToggle(ToggleButton *array[], uint8_t amount);
#endif

#if defined(DFK_TOUCHAREA)
    void setTouchArea(TouchArea *array[], uint8_t amount);
#endif

#ifdef DFK_VANALOG
    void setVAnalog(VAnalog *array[], uint8_t amount);
#endif

#ifdef DFK_VBAR
    void setVBar(VBar *array[], uint8_t amount);
#endif

#ifdef DFK_IMAGE
    void setImage(Image *array[], uint8_t amount);
#endif

#ifdef DFK_SPINBOX
    void setSpinbox(SpinBox *array[], uint8_t amount);
#endif

#if defined(DFK_TEXTBUTTON)
    void setTextButton(TextButton *array[], uint8_t amount);
#endif

#ifdef DFK_THERMOMETER
    void setThermometer(Thermometer *array[], uint8_t amount);
#endif

#ifdef DFK_CAMERA_MIPI
    void setCameraMipi(CameraMipi *array[], uint8_t amount);
#endif

#ifdef DFK_SD
    bool startSD(uint8_t pinCS, SPIClass *spiShared);
    bool startSD(uint8_t pinCS, SPIClass *spiShared, int hz);
    void listFiles(fs::FS *fs, const char *dirname, uint8_t levels);
    void createDir(fs::FS *fs, const char *path);
    void readFile(fs::FS *fs, const char *path);
    void appendFile(fs::FS *fs, const char *path, const char *message);
    void appendLog(fs::FS *fs, const char *path, const logMessage_t *lines, uint8_t amount, bool createNewFile);
    void writeFile(fs::FS *fs, const char *path, const char *message);
    const char *getLogFileName() const;
    const char *generateNameFile() const;
#endif

private:
    static const char *TAG;
    // Variáveis privadas estáticas
    static QueueHandle_t xFilaLog;             ///< Queue handle for log data.
    static logMessage_t bufferLog[LOG_LENGTH]; ///< Array of log messages.
    static uint8_t logIndex;                   ///< Index of the next log message to be written.
    static uint16_t logFileCount;              ///< Number of log files created
    static StringPool stringPool;              ///< Pool for temporary string allocations
    static int offsetSwipe; /// offsetSwipe minutes for detect right/left swipe

#ifdef DFK_EXTERNALINPUT
    static uint8_t qtdExternalInput;           ///< Number of ExternalInput widgets.
    static InputExternal *arrayInputExternal;  ///< Array of ExternalInput widgets.
    static ExternalKeyboard *externalKeyboard; ///< Pointer to the Numpad instance for number input.
#endif

    // Variáveis privadas
#if defined(HAS_CAMERA_MIPI) && defined(DFK_CAMERA_MIPI)
    CameraController m_camera;
    uint8_t m_cameraRotation;
    bool m_cameraMirrorX;
    bool m_cameraMirrorY;
#if defined(CONFIG_IDF_TARGET_ESP32P4)
    Arduino_DSI_Display *m_cameraDsiDisplay;
#endif
    uint16_t *m_cameraPreviewFb;
    size_t m_cameraPreviewFbBytes;
    uint8_t m_cameraPreviewFps = 5;
    uint16_t *m_frameCopy = nullptr;
    size_t m_frameCopyBytes = 0;
    uint32_t m_frameCopyW = 0;
    uint32_t m_frameCopyH = 0;
    std::atomic<uint8_t> m_cameraCopyPhase{0};
    TaskHandle_t m_frameCopyWaiter = nullptr;
    std::unique_ptr<CameraRoi> m_cameraRoi;
    bool m_cameraRoiActive = false;
    bool m_cameraRoiTouchWasDown = false;
    bool m_seenCameraFullScreen = false;
    bool m_fullScreenBarsPending = false;
    uint32_t m_fullScreenFpsWindowMs = 0;
    uint16_t m_fullScreenFpsCount = 0;
    uint8_t m_fullScreenFpsShown = 0;
    TaskHandle_t m_hndTaskCamera = nullptr;
    std::atomic<bool> m_cameraTaskStopped{true};
#endif
    TaskHandle_t m_hndTaskEventoTouch; ///< Handle for the touch event task.
    TaskHandle_t m_stopWaiter = nullptr;
    std::atomic<bool> m_stopTask{false};
    std::atomic<bool> m_taskStopped{false};
    bool m_runningTransaction;
    Preferences m_configs; ///< Preferences for storing and accessing configuration data.
    TimerHandle_t m_timer;
    uint32_t m_intervalMs;
    uint16_t m_xAutoClick = 0;
    uint16_t m_yAutoClick = 0;
    bool m_simulateAutoClick = false;
    bool m_ignoreOpeningTouch = false;
    bool m_debugTouch = false;
    int m_x0 = 0, m_y0 = 0, m_x1 = 0, m_y1 = 0;
    bool m_invertXAxis = false, m_invertYAxis = false, m_swapAxis = false;
    int m_widthScreen = 0;
    int m_heightScreen = 0;
    int m_rotationScreen = 0;
    uint16_t m_timeoutWTD = 0;
    bool m_enableWTD = false;
    bool m_watchdogInitialized = false;
    SemaphoreHandle_t m_loopSemaphore;
    SemaphoreHandle_t m_transactionSemaphore;
    TouchEventType m_lastTouchState = TouchEventType::NONE;
    TouchSwipeDirection m_lastTouchSwipeDirection = TouchSwipeDirection::NONE;
    CoordPoint_t pressPoint = {x: 0, y: 0};
    CoordPoint_t releasePoint = {x: 0, y: 0};
    static void logHeapPorTipo(const char *nome, uint32_t capacidades);
    static void logMemoriaSistema();

#ifdef ENABLE_ON_RELEASE
    // Touch tracking for onRelease functionality
    WidgetBase* m_touchedWidgets[5];      ///< Array of widgets currently being touched
    uint8_t m_touchedWidgetsCount;        ///< Number of widgets in the touched array
    bool m_trackingTouch;                 ///< Flag indicating if we're tracking touch
#endif

#if defined(DFK_TOUCHAREA)
    uint8_t qtdTouchArea = 0;             ///< Number of TouchArea widgets.
    TouchArea **arrayTouchArea = nullptr; ///< Array of TouchArea widgets.
    bool m_touchAreaConfigured = false;   ///< Flag indicating if TouchArea is configured.
#endif

#ifdef DFK_CHECKBOX
    uint8_t qtdCheckbox = 0;            ///< Number of CheckBox widgets.
    CheckBox **arrayCheckbox = nullptr; ///< Array of CheckBox widgets.
    bool m_checkboxConfigured = false;  ///< Flag indicating if CheckBox is configured.
#endif

#ifdef DFK_CIRCLEBTN
    uint8_t qtdCircleBtn = 0;                ///< Number of CircleButton widgets.
    CircleButton **arrayCircleBtn = nullptr; ///< Array of CircleButton widgets.
    bool m_circleButtonConfigured = false;   ///< Flag indicating if CircleButton is configured.
#endif

#ifdef DFK_GAUGE
    uint8_t qtdGauge = 0;              ///< Number of GaugeSuper widgets.
    GaugeSuper **arrayGauge = nullptr; ///< Array of GaugeSuper widgets.
    bool m_gaugeConfigured = false;    ///< Flag indicating if GaugeSuper is configured.
#endif

#ifdef DFK_CIRCULARBAR
    uint8_t qtdCircularBar = 0;               ///< Number of CircularBar widgets.
    CircularBar **arrayCircularBar = nullptr; ///< Array of CircularBar widgets.
    bool m_circularBarConfigured = false;     ///< Flag indicating if CircularBar is configured.
#endif

#ifdef DFK_HSLIDER
    uint8_t qtdHSlider = 0;           ///< Number of HSlider widgets.
    HSlider **arrayHSlider = nullptr; ///< Array of HSlider widgets.
    bool m_hSliderConfigured = false; ///< Flag indicating if HSlider is configured.
#endif

#ifdef DFK_LABEL
    uint8_t qtdLabel = 0;           ///< Number of Label widgets.
    Label **arrayLabel = nullptr;   ///< Array of Label widgets.
    bool m_labelConfigured = false; ///< Flag indicating if Label is configured.
#endif

#ifdef DFK_LED
    uint8_t qtdLed = 0;           ///< Number of Led widgets.
    Led **arrayLed = nullptr;     ///< Array of Led widgets.
    bool m_ledConfigured = false; ///< Flag indicating if Led is configured.
#endif

#ifdef DFK_LINECHART
    uint8_t qtdLineChart = 0;             ///< Number of LineChart widgets.
    LineChart **arrayLineChart = nullptr; ///< Array of LineChart widgets.
    bool m_lineChartConfigured = false;   ///< Flag indicating if LineChart is configured.
#endif

#ifdef DFK_RADIO
    uint8_t qtdRadioGroup = 0;              ///< Number of RadioGroup widgets.
    RadioGroup **arrayRadioGroup = nullptr; ///< Array of RadioGroup widgets.
    bool m_radioGroupConfigured = false;    ///< Flag indicating if RadioGroup is configured.
#endif

#ifdef DFK_RECTBTN
    uint8_t qtdRectBtn = 0;              ///< Number of RectButton widgets.
    RectButton **arrayRectBtn = nullptr; ///< Array of RectButton widgets.
    bool m_rectButtonConfigured = false; ///< Flag indicating if RectButton is configured.
#endif

#ifdef DFK_TOGGLE
    uint8_t qtdToggle = 0;                   ///< Number of ToggleButton widgets.
    ToggleButton **arrayToggleBtn = nullptr; ///< Array of ToggleButton widgets.
    bool m_toggleConfigured = false;         ///< Flag indicating if ToggleButton is configured.
#endif

#ifdef DFK_VBAR
    uint8_t qtdVBar = 0;           ///< Number of VBar widgets.
    VBar **arrayVBar = nullptr;    ///< Array of VBar widgets.
    bool m_vBarConfigured = false; ///< Flag indicating if VBar is configured.
#endif

#ifdef DFK_VANALOG
    uint8_t qtdVAnalog = 0;           ///< Number of VAnalog widgets.
    VAnalog **arrayVAnalog = nullptr; ///< Array of VAnalog widgets.
    bool m_vAnalogConfigured = false; ///< Flag indicating if VAnalog is configured.
#endif

#ifdef DFK_TEXTBOX
    uint8_t qtdTextBox = 0;              ///< Number of TextBox widgets.
    TextBox **arrayTextBox = nullptr;    ///< Array of TextBox widgets.
    bool m_textboxConfigured = false;    ///< Flag indicating if TextBox is configured.
    std::unique_ptr<WKeyboard> keyboard; ///< Pointer to the WKeyboard instance for text input.
    // WKeyboard m_pKeyboard;            ///< Internal keyboard instance for TextBox.
#endif

#ifdef DFK_NUMBERBOX
    uint8_t qtdNumberBox = 0;             ///< Number of NumberBox widgets.
    NumberBox **arrayNumberbox = nullptr; ///< Array of NumberBox widgets.
    bool m_numberboxConfigured = false;   ///< Flag indicating if NumberBox is configured.
    bool m_numpadActive = false;
    bool m_numpadTouchWasDown = false;
    std::unique_ptr<Numpad> numpad;       ///< Pointer to the Numpad instance for number input.
    // Numpad m_pNumpad;                   ///< Internal numpad instance for NumberBox.
#endif

#ifdef DFK_IMAGE
    uint8_t qtdImage = 0;           ///< Number of Image widgets.
    Image **arrayImage = nullptr;   ///< Array of Image widgets.
    bool m_imageConfigured = false; ///< Flag indicating if Image is configured.
#endif

#ifdef DFK_TEXTBUTTON
    uint8_t qtdTextButton = 0;              ///< Number of TextButton widgets.
    TextButton **arrayTextButton = nullptr; ///< Array of TextButton widgets.
    bool m_textButtonConfigured = false;    ///< Flag indicating if TextButton is configured.
#endif

#ifdef DFK_SD
    SPIClass m_spiSD;          ///< SPI instance for SD card communication.
    const char *m_nameLogFile; ///< Name of the log file.
#endif

#ifdef DFK_SPINBOX
    uint8_t qtdSpinbox = 0;           ///< Number of SpinBox widgets.
    SpinBox **arraySpinbox = nullptr; ///< Array of SpinBox widgets.
    bool m_spinboxConfigured = false; ///< Flag indicating if SpinBox is configured.
#endif

#ifdef DFK_THERMOMETER
    uint8_t qtdThermometer = 0;               ///< Number of Thermometer widgets.
    Thermometer **arrayThermometer = nullptr; ///< Array of Thermometer widgets.
    bool m_thermometerConfigured = false;     ///< Flag indicating if Thermometer is configured.
#endif

#ifdef DFK_CAMERA_MIPI
    uint8_t qtdCameraMipi = 0;            ///< Number of Camera widgets.
    CameraMipi **arrayCameraMipi = nullptr; ///< Array of Camera widgets.
    bool m_cameraMipiConfigured = false;   ///< Flag indicating if Camera is configured.
#endif

#ifdef DFK_EXTERNALINPUT
    bool m_inputExternalConfigured = false; ///< Flag indicating if ExternalInput is configured.
    ExternalKeyboard m_pExternalKeyboard;   ///< Internal numpad instance for NumberBox.
#endif

    functionLoadScreen_t m_lastScreen = nullptr;

    // Métodos privados estáticos
    static void timerCallback(TimerHandle_t xTimer);

    // Métodos privados
    char randomHexChar();

    // Helper functions for displayfkTask
    void processLogQueue();
    void processLogBuffer();
    void processTouchEvent(uint16_t xTouch, uint16_t yTouch, int zPressure, uint8_t gesture);
    void processTouchStatus(bool hasTouch, uint16_t xTouch, uint16_t yTouch);
    void processTouchableWidgets(uint16_t xTouch, uint16_t yTouch, bool collectMode = false);
    void updateWidgets();
    void processCallback();

    // Touch processing functions
    void processCheckboxTouch(uint16_t xTouch, uint16_t yTouch, bool collectMode = false);
    void processCircleButtonTouch(uint16_t xTouch, uint16_t yTouch, bool collectMode = false);
    void processHSliderTouch(uint16_t xTouch, uint16_t yTouch, bool collectMode = false);
    void processRadioGroupTouch(uint16_t xTouch, uint16_t yTouch, bool collectMode = false);
    void processRectButtonTouch(uint16_t xTouch, uint16_t yTouch, bool collectMode = false);
    void processImageTouch(uint16_t xTouch, uint16_t yTouch, bool collectMode = false);
    void processSpinboxTouch(uint16_t xTouch, uint16_t yTouch, bool collectMode = false);
    void processToggleTouch(uint16_t xTouch, uint16_t yTouch, bool collectMode = false);
    void processTextButtonTouch(uint16_t xTouch, uint16_t yTouch, bool collectMode = false);
    void processTextBoxTouch(uint16_t xTouch, uint16_t yTouch, bool collectMode = false);
    void processNumberBoxTouch(uint16_t xTouch, uint16_t yTouch, bool collectMode = false);
    void serviceKeyboard(bool hasTouch, uint16_t xTouch, uint16_t yTouch);
    void serviceNumpad(bool hasTouch, uint16_t xTouch, uint16_t yTouch);
    void processEmptyAreaTouch(uint16_t xTouch, uint16_t yTouch, bool collectMode = false);
    void processCameraMipiTouch(uint16_t xTouch, uint16_t yTouch, bool collectMode = false);
    bool processTouchOnCameraFullScreen();
    void exitCameraFullScreen();

#ifdef ENABLE_ON_RELEASE
    // Touch tracking helper methods
    void addTouchedWidget(WidgetBase* widget);
    void removeTouchedWidget(WidgetBase* widget);
    bool isWidgetInTouchedArray(WidgetBase* widget) const;
    void checkWidgetsStillTouched(uint16_t xTouch, uint16_t yTouch);
    void clearTouchedWidgets(bool callOnRelease = true);
#endif

    // Widget update functions
    void updateCircularBar();
    void updateGauge();
    void updateLabel();
    void updateLed();
    void updateLineChart();
    void updateVBar();
    void updateVAnalog();
    void updateCheckbox();
    void updateCircleButton();
    void updateHSlider();
    void updateRadioGroup();
    void updateRectButton();
    void updateToggle();
    void updateImage();
    void updateTextButton();
    void updateSpinbox();
    void updateTextBox();
    void updateNumberBox();
    void updateThermometer();
    #if defined(DFK_CAMERA_MIPI) && defined(HAS_CAMERA_MIPI)
    friend class CameraRoi;
    void updateCameraMipi(ESPVideoBufferClass *buffer);
    bool blitCameraDmaFullScreen(const uint16_t *src, uint32_t cam_w, uint32_t cam_h);
    bool blitCameraDmaCropped(const uint16_t *src, uint32_t cam_w, uint32_t cam_h, int x, int y, uint32_t w, uint32_t h, uint32_t crop_x, uint32_t crop_y, uint32_t crop_w, uint32_t crop_h, uint32_t scale_num);
    bool ensureCameraPreview(uint32_t w, uint32_t h);
    void drawCameraOverlay(CameraMipi *widget, bool fullScreen, bool showFps);
    uint32_t cameraPreviewWaitMs() const;
    void serviceCameraRoi(bool hasTouch, uint16_t xTouch, uint16_t yTouch);
    void finishCameraConfig(bool apply);
    bool requestCameraFrameCopy();
    void releaseCameraFrameCopy();
    void publishCameraFrameCopy(const uint16_t *src, uint32_t width, uint32_t height);
    bool clampCameraRoiCenter(uint32_t frameW, uint32_t frameH, uint32_t plotW, uint32_t plotH, int16_t *centerX, int16_t *centerY, uint8_t zoom, uint32_t *cropW, uint32_t *cropH) const;
    bool drawFrozenCamera(CameraMipi *widget, int16_t centerX, int16_t centerY, uint8_t zoom, int areaX, int areaY, int areaW, int areaH, CameraConfigView *view);
    #else
    void updateCameraMipi();
    #endif
};

#endif
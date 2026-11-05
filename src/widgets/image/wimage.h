#ifndef WIMAGE
#define WIMAGE

#include "../widgetbase.h"

#if defined(USE_SPIFFS)
#include "SPIFFS.h"
#endif
#if defined(USE_FATFS)
#include "FFat.h"
#endif

#include <FS.h>

/// @brief Enum for specifying the source file location of the image.
enum class SourceFile {
  SD = 0,     ///< Image source from SD card.
  SPIFFS = 1, ///< Image source from SPIFFS filesystem.
  EMBED = 2,   ///< Image source embedded within the code.
  FATFS = 3 ///< Image source from FAT filesystem
};

#if defined(DISP_DEFAULT)
  typedef uint16_t pixel_t;
#else
  typedef uint8_t pixel_t;
#endif
/**
 * @brief Configuration structure for loading an image from a file.
 *
 * This structure defines the configuration for loading an image from a file.
 * It includes the source file location, the path to the image file,
 * the callback function to be called when the image is touched,
 * and the rotation angle of the image.
 */
struct ImageFromFileConfig {
  const char *path;  ///< Path to the image file.
  functionCB_t cb; ///< Callback function to be called when the image is touched.
  SourceFile source; ///< Source of the image file.
  uint16_t backgroundColor; ///< Background color of the image.
  String toString(){
    return String((int)source) + " " + path;
  }
};

/**
 * @brief Configuration structure for loading an image from pixel data.
 *
 * This structure defines the configuration for loading an image from pixel data.
 * It includes the pixel data, dimensions, alpha mask, and callback function.
 * 
 * @note The pixel data format depends on the display type:
 *       - DISP_DEFAULT: 16-bit RGB565 format
 *       - DISP_PCD8544/SSD1306/U8G2: 8-bit monochrome format
 */
struct ImageFromPixelsConfig {
  const pixel_t *pixels;        ///< Pointer to the pixel map data (format depends on display type)
  const uint8_t *maskAlpha;     ///< Pointer to the alpha mask data for transparency (optional)
  functionCB_t cb;              ///< Callback function to be called when the image is touched
  float angle;                  ///< Rotation angle of the image in degrees (0.0 = no rotation)
  uint16_t width;               ///< Width of the image (must be > 0)
  uint16_t height;              ///< Height of the image (must be > 0)
  uint16_t backgroundColor;      ///< Background color of the image
  
  /**
   * @brief Validates the configuration structure
   * @return true if configuration is valid, false otherwise
   */
  bool isValid() const {
    if (pixels == nullptr) return false;
    if (width == 0 || height == 0) return false;
    if (width > 4096 || height > 4096) return false; // Reasonable limits
    return true;
  }
  
  /**
   * @brief Gets the total number of pixels
   * @return Total pixel count
   */
  uint32_t getPixelCount() const {
    return static_cast<uint32_t>(width) * static_cast<uint32_t>(height);
  }
};
typedef struct  {
  uint32_t drawCount = 0;           ///< Number of times draw() was called
  uint32_t fileLoadCount = 0;       ///< Number of times file was loaded
  uint32_t rotationDrawCount = 0;   ///< Number of times rotated image was drawn
  uint32_t totalDrawTime = 0;       ///< Total time spent in draw() (microseconds)
  uint32_t totalFileLoadTime = 0;   ///< Total time spent loading files (microseconds)
  uint32_t totalRotationTime = 0;   ///< Total time spent in rotation (microseconds)
  uint32_t maxDrawTime = 0;         ///< Maximum single draw time (microseconds)
  uint32_t maxFileLoadTime = 0;     ///< Maximum single file load time (microseconds)
  uint32_t maxRotationTime = 0;     ///< Maximum single rotation time (microseconds)
  uint32_t lastDrawTime = 0;        ///< Last draw time (microseconds)
  uint32_t lastFileLoadTime = 0;    ///< Last file load time (microseconds)
  uint32_t lastRotationTime = 0;    ///< Last rotation time (microseconds)
  
  void reset() {
    drawCount = 0;
    fileLoadCount = 0;
    rotationDrawCount = 0;
    totalDrawTime = 0;
    totalFileLoadTime = 0;
    totalRotationTime = 0;
    maxDrawTime = 0;
    maxFileLoadTime = 0;
    maxRotationTime = 0;
    lastDrawTime = 0;
    lastFileLoadTime = 0;
    lastRotationTime = 0;
  }
  
  float getAverageDrawTime() const {
    return drawCount > 0 ? (float)totalDrawTime / drawCount : 0.0f;
  }
  
  float getAverageFileLoadTime() const {
    return fileLoadCount > 0 ? (float)totalFileLoadTime / fileLoadCount : 0.0f;
  }
  
  float getAverageRotationTime() const {
    return rotationDrawCount > 0 ? (float)totalRotationTime / rotationDrawCount : 0.0f;
  }
} PerformanceMetrics_t;

/// @brief Widget de imagem que pode carregar imagens de várias fontes.
/// @details Esta classe herda de @ref WidgetBase e fornece funcionalidade completa para criar e gerenciar 
///          imagens interativas na tela. O Image pode carregar imagens de arquivos (SD, SPIFFS, FATFS) ou
///          de dados de pixels embutidos no código. Suporta rotação, máscaras de transparência, callbacks
///          de toque e métricas de desempenho para otimização. O widget é totalmente funcional com suporte
///          a diferentes formatos de display (RGB565, monocromático) e gerenciamento inteligente de memória.
class Image : public WidgetBase {
public:
  Image(uint16_t _x, uint16_t _y, uint8_t _screen);

  ~Image();

  bool detectTouch(uint16_t *_xTouch, uint16_t *_yTouch) override;

  functionCB_t getCallbackFunc() override;

  void redraw() override;

  void drawBackground();

  void draw();

  void show() override;
  void hide() override;

  void forceUpdate() override;

  void setupFromFile(ImageFromFileConfig &config);
  bool setupFromFileSafe(ImageFromFileConfig &config);
  void setupFromPixels(ImageFromPixelsConfig &config);

  void setDrawBackground(bool value);
  
  // Performance metrics access
  const PerformanceMetrics_t& getMetrics() const { return m_metrics; }
  void resetMetrics() { m_metrics.reset(); }
  void printMetrics() const;

private:
  static const char* TAG; ///< Tag estática para identificação em logs do ESP32.
  ImageFromPixelsConfig m_config; ///< Configuração unificada da imagem (fonte única de verdade).
  bool m_ownsMemory; ///< Flag para controlar propriedade da memória (true = alocamos, false = referência externa).
  pixel_t *m_pixels; ///< Ponteiro para dados de pixels (legacy para compatibilidade).
  uint8_t *m_maskAlpha; ///< Ponteiro para máscara de transparência (legacy para compatibilidade).
  SourceFile m_source; ///< Fonte do arquivo de imagem (legacy para compatibilidade).
  fs::FS *m_fs; ///< Ponteiro para sistema de arquivos (legacy para compatibilidade).
  const char *m_path; ///< Caminho para o arquivo de imagem (legacy para compatibilidade).
  PerformanceMetrics_t m_metrics; ///< Métricas de desempenho para otimização.
  bool m_drawBackground; ///< Flag para controlar desenho do fundo (true = desenha, false = não desenha).
  
  bool readFileFromDisk(fs::FS &filesystem, const char *path);
  void drawUnpackedMonoPixels();
  void defineFileSystem(SourceFile source);
  void drawRotatedImage();
  bool validateConfig();
  void clearBuffers();
  void listDir(fs::FS &fs, const char * dirname);
};

#endif
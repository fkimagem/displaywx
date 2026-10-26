#include "../user_setup.h"
#include "widgets/widgetsetup.h"


#ifdef DFK_GAUGE
#include "widgets/gauge/wgaugesuper.h"
#endif

#ifdef DFK_CIRCULARBAR
#include "widgets/circularbar/wcircularbar.h"
#endif

#ifdef DFK_LABEL
#include "widgets/label/wlabel.h"
#endif

#ifdef DFK_LED
#include "widgets/led/wled.h"
#endif

#ifdef DFK_LINECHART
#include "widgets/linechart/wlinechart.h"
#endif

#ifdef DFK_VBAR
#include "widgets/verticalbar/wverticalbar.h"
#endif

#ifdef DFK_VANALOG
#include "widgets/verticalanalog/wverticalanalog.h"
#endif

#ifdef DFK_IMAGE
#include "widgets/image/wimage.h"
#endif

#ifdef DFK_THERMOMETER
#include "widgets/thermometer/wthermometer.h"
#endif

#ifdef DFK_CAMERA_MIPI
#include "widgets/cameramipi/wcameramipi.h"
#if defined(HAS_CAMERA_MIPI)
#include "widgets/cameramipi/cameraroi.h"
#endif
#endif

#ifdef DFK_EXTERNALINPUT
#include "widgets/externalinput/winputexternal.h"
#include "widgets/externalinput/wexternalkeyboard.h"
#endif
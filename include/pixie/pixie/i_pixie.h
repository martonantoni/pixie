#include "Window/i_window.h"

#include <math.h>

#define PIXIE_ARGB(a, r, g, b) \
    ((uint32_t(a) << 24) | \
     (uint32_t(r) << 16) | \
     (uint32_t(g) << 8)  | \
      uint32_t(b))

#define PIXIE_XRGB(r, g, b) \
    PIXIE_ARGB(0xff, r, g, b)

#include "system/ColorInfo.h"
#include "system/PixieEvents.h"
#include "system/LogicServer.h"
#include "system/MouseServer.h"
#include "system/KeyboardServer.h"
#include "system/PixieInitializer.h"
#include "system/Spline.h"
#include "system/startup_controller.h"


#include "FreeTypeIntegration/FreeTypeIntegration.h"

#include "PixieObject/i_PixieObject.h"

#include "3dEngine/i_3DEngine.h"
#include "2dEngine/i_2DEngine.h" // uses 3d engine

#include "Sound/i_Sound.h"

#include "UI/i_ui.h" // uses both sound & 2d engine

#include "Editor/i_Editor.h"

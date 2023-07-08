#include "Window/i_window.h"

#include <d3d9.h>
#include <d3dx9.h>
#include <math.h>

#include "system/PixieCommon.h"
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

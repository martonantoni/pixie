#include "StdAfx.h"
#include "pixie/pixie/i_pixie.h"
#include "pixie\pixie\System\PixieMainLoop.h"

void registerGlobalPixieLuaFunctions(cLuaObject globalTable)
{
	globalTable.registerFunction("RGB",
		[](int red, int green, int blue) -> int
		{
			ASSERT(red >= 0 && red <= 255);
			ASSERT(green >= 0 && green <= 255);
			ASSERT(blue >= 0 && blue <= 255);

			uint32_t color =
				(0xffu << 24) |
				(static_cast<uint32_t>(red) << 16) |
				(static_cast<uint32_t>(green) << 8) |
				static_cast<uint32_t>(blue);

			return static_cast<int>(color);
		});

	globalTable.registerFunction("ARGB",
		[](int alpha, int red, int green, int blue) -> int
		{
			ASSERT(alpha >= 0 && alpha <= 255);
			ASSERT(red >= 0 && red <= 255);
			ASSERT(green >= 0 && green <= 255);
			ASSERT(blue >= 0 && blue <= 255);

			uint32_t color =
				(static_cast<uint32_t>(alpha) << 24) |
				(static_cast<uint32_t>(red) << 16) |
				(static_cast<uint32_t>(green) << 8) |
				static_cast<uint32_t>(blue);

			return static_cast<int>(color);
		});
	globalTable.registerFunction("XEnd",
		[](cLuaObject object) -> int
		{
			return object.get<int>("x") + object.get<int>("w");
		});
    globalTable.registerFunction("YEnd",
        [](cLuaObject object) -> int
        {
            return object.get<int>("y") + object.get<int>("h");
        });
	globalTable.registerFunction("Rect",
		[](cLuaState script, int x, int y, int w, int h) -> cLuaObject
		{
			cLuaObject rectTable = script.createTable();
			rectTable.set("x", x);
			rectTable.set("y", y);
			rectTable.set("w", w);
			rectTable.set("h", h);
			return rectTable;
		});


	globalTable.state().executeString(
		"function Get9PatchNames()\n"
		"return {"
		"\"top_left\", \"top\", \"top_right\","
		"\"left\", \"middle\", \"right\","
		"\"bottom_left\", \"bottom\", \"bottom_right\" }\n"
	    "end\n");
}


void InitPixieSystem()
{
	cPrimaryWindow::get();
	theColorServer.Init();
	(new cBasicDeviceClearer)->Init(cConfig());
	cDevice::Get();
    cShaderManager::get();
    theShaderManager->init();
	thePixieDesktop.Init(cPixieDesktop::cInitData());
	theTextureManager.Initialize();
    theSoundPlayer.Initialize();
	InitFreeType();
	cPixieObjectAnimatorManager::get();
	cMouseServer::get();
	cKeyboardServer::get();
	cMouseCursorServer::get();
	thePixieSystem.init();
}
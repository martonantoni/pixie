#include "StdAfx.h"
#include "pixie/pixie/i_pixie.h"
#include "pixie\pixie\System\PixieMainLoop.h"

/*
int LUA_Bridge_CreateRGBValue(int red,int green,int blue)
{
	ASSERT(red>=0&&red<=255);
	ASSERT(green>=0&&green<=255);
	ASSERT(blue>=0&&blue<=255);
	return D3DCOLOR_ARGB(0xff,red,green,blue);
}
REGISTER_LUA_FUNCTION(LUA_Bridge_CreateRGBValue,RGB);
int LUA_Bridge_CreateARGBValue(int alpha,int red,int green,int blue)
{
	ASSERT(alpha>=0&&alpha<=255);
	ASSERT(red>=0&&red<=255);
	ASSERT(green>=0&&green<=255);
	ASSERT(blue>=0&&blue<=255);
	return D3DCOLOR_ARGB(alpha,red,green,blue);
}
REGISTER_LUA_FUNCTION(LUA_Bridge_CreateARGBValue, ARGB);

int LUA_Bridge_XEnd(LuaObject Object)
{
	if(ASSERTFALSE(!Object.IsTable()))
		return 0;
	return Object.GetByName("x").GetInteger()+Object.GetByName("w").GetInteger();
}
int LUA_Bridge_YEnd(LuaObject Object)
{
	if(ASSERTFALSE(!Object.IsTable()))
		return 0;
	return Object.GetByName("y").GetInteger()+Object.GetByName("h").GetInteger();
}
REGISTER_LUA_FUNCTION(LUA_Bridge_XEnd, XEnd);
REGISTER_LUA_FUNCTION(LUA_Bridge_YEnd, YEnd);

LuaObject LUA_Bridge_Rect(int x, int y, int w, int h)
{
	LuaObject Table=theLuaState->CreateTable();
	Table.Set("x", x);
	Table.Set("y", y);
	Table.Set("w", w);
	Table.Set("h", h);
	return Table;
}
REGISTER_LUA_FUNCTION(LUA_Bridge_Rect, Rect);
*/

void registerGlobalPixieLuaFunctions(cLuaValue globalTable)
{
	globalTable.registerFunction<int, int, int, int>("RGB",
		[](int red, int green, int blue) -> int
		{
            ASSERT(red >= 0 && red <= 255);
            ASSERT(green >= 0 && green <= 255);
            ASSERT(blue >= 0 && blue <= 255);
            return D3DCOLOR_ARGB(0xff, red, green, blue);
		});
    globalTable.registerFunction<int, int, int, int, int>("ARGB",
        [](int alpha, int red, int green, int blue) -> int
        {
            ASSERT(alpha >= 0 && alpha <= 255);
            ASSERT(red >= 0 && red <= 255);
            ASSERT(green >= 0 && green <= 255);
            ASSERT(blue >= 0 && blue <= 255);
            return D3DCOLOR_ARGB(alpha, red, green, blue);
        });
	globalTable.registerFunction<int, cLuaValue>("XEnd",
		[](cLuaValue object) -> int
		{
			return *object.get<int>("x") + *object.get<int>("w");
		});
    globalTable.registerFunction<int, cLuaValue>("YEnd",
        [](cLuaValue object) -> int
        {
            return *object.get<int>("y") + *object.get<int>("h");
        });
	globalTable.registerFunction<cLuaValue, cLuaScript&, int, int, int, int>("Rect",
		[](cLuaScript& script, int x, int y, int w, int h) -> cLuaValue
		{
			cLuaValue rectTable = script.createTable();
			rectTable.set("x", x);
			rectTable.set("y", y);
			rectTable.set("w", w);
			rectTable.set("h", h);
			return rectTable;
		});


	globalTable.script().executeString(
		"function Get9PatchNames()\n"
		"return {"
		"\"top_left\", \"top\", \"top_right\","
		"\"left\", \"middle\", \"right\","
		"\"bottom_left\", \"bottom\", \"bottom_right\" }\n"
	    "end\n");

//	globalTable.

    // LuaObject cMultiSprite_Simple9Patch::Lua_Get9PatchNames()
// {
// 	LuaObject Table=theLuaState->CreateTable();
// 	static const char *PositionNames[]= {
// 		"top_left", "top", "top_right",
// 		"left", "middle", "right",   
// 		"bottom_left", "bottom", "bottom_right",
// 		nullptr };
// 	for(int i=0; PositionNames[i]; ++i)
// 	{
// 		Table.Set(i+1, PositionNames[i]);
// 	}
// 	return Table;
// }
// REGISTER_LUA_FUNCTION(cMultiSprite_Simple9Patch::Lua_Get9PatchNames, Get9PatchNames);

}


void InitPixieSystem()
{
	cPrimaryWindow::Get();
	theColorServer.Init();
	(new cBasicDeviceClearer)->Init(cConfig2());
	cDevice::Get();
	thePixieDesktop.Init(cPixieDesktop::cInitData());
	theTextureManager.Initialize();
    theSoundPlayer.Initialize();
	InitFreeType();
	cPixieObjectAnimatorManager::Get();
	cMouseServer::Get();
	cKeyboardServer::Get();
	InitPixieSystemMainLoop();
}
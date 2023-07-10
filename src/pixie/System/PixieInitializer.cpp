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

void registerGlobalPixieLuaFunctions(cLuaTable globalTable)
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
	globalTable.registerFunction<int, cLuaTable>("XEnd",
		[](cLuaTable object) -> int
		{
			return object.get<int>("x") + object.get<int>("w");
		});
    globalTable.registerFunction<int, cLuaTable>("YEnd",
        [](cLuaTable object) -> int
        {
            return object.get<int>("y") + object.get<int>("h");
        });
	globalTable.registerFunction<cLuaTable, cLuaScript&, int, int, int, int>("Rect",
		[](cLuaScript& script, int x, int y, int w, int h) -> cLuaTable
		{
			cLuaTable rectTable = script.createTable();
			rectTable.set("x", x);
			rectTable.set("y", y);
			rectTable.set("w", w);
			rectTable.set("h", h);
			return rectTable;
		});
}


void InitPixieSystem()
{
	theColorServer.Init();
	cEventCenter::Get();
	(new cBasicDeviceClearer)->Init(cEmptyConfig());
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
#include "StdAfx.h"
#include "pixie/pixie/i_pixie.h"
/*
cTextureManager::cTextureManager()
{
	mDevice=cDevice::Get();
	mTextureInfoRoot=Expression::cObject::GetObject_FullRoute("TextureData",false);
	RELEASE_ASSERT(mTextureInfoRoot);
}

cTextureManager::~cTextureManager()
{
}

cTexture *cTextureManager::GetTextureByName(const std::string &TextureName)
{
	cTexture *&Texture=mTextureMap[TextureName];
	if(!Texture)
	{
		Expression::cObject *TextureInfo=mTextureInfoRoot->GetSubObject(TextureName,false);
		RELEASE_ASSERT_EXT(TextureInfo,Format("Texture info missing for: \"%s\"",TextureName));
		std::string TextureFileName=TextureInfo->GetString("FileName");
		cTexture *BaseTexture=GetTextureByFileName(TextureFileName);
		Texture=new cTexture(BaseTexture,TextureInfo);
	}
	return Texture;
}

cTexture *cTextureManager::GetTextureByFileName(const std::string &TextureFileName)
{
	cTexture *&Texture=mTextureFileMap[TextureFileName];
	if(!Texture)
	{
		D3DXIMAGE_INFO SrcInfo;      //Optional
		IDirect3DTexture9 *Direct3DTexture=LoadTexture(TextureFileName,&SrcInfo);
		RELEASE_ASSERT_EXT(Direct3DTexture,Format("Failed to load texture file: \"%s\"",TextureFileName));
		Texture=new cTexture(Direct3DTexture,SrcInfo.Width,SrcInfo.Height);
	}
	return Texture;
}

void cTextureManager::NoLongerReferenced(cTexture *Texture)
{
	// current implementation: do nothing
}

//Load texture from file with D3DX
//Supported formats: BMP, PPM, DDS, JPG, PNG, TGA, DIB
IDirect3DTexture9 *cTextureManager::LoadTexture(const std::string &FileName,D3DXIMAGE_INFO *SrcInfo)
{
	IDirect3DTexture9 *d3dTexture;

//Use a magenta colourkey
	D3DCOLOR colorkey = 0xFFFF00FF;

// Load image from file
	if (FAILED(D3DXCreateTextureFromFileEx (*theDevice, (const char *)FileName,
		D3DX_DEFAULT_NONPOW2 , D3DX_DEFAULT_NONPOW2 ,
		
//		0, 0, 
		1, 0, 
        D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, D3DX_FILTER_NONE, D3DX_DEFAULT, 
        colorkey, SrcInfo, NULL, &d3dTexture)))
	{
		int Err=GetLastError();
		MainLog->Log("Failed to load texture file: %s, Error: %d",FileName,Err);
		return NULL;
	}

//Return the newly made texture
	return d3dTexture;
}
*/
#pragma once

class cTextureManager: public tSingleton<cTextureManager>
{
	cDevice *mDevice;
	typedef std::map<std::string,cTexture *> cTextureFileMap;
	cTextureFileMap mTextureFileMap; // filename -> texture
	typedef std::map<std::string,cTexture *> cTextureMap;
	cTextureMap mTextureMap;
	Expression::cObject *mTextureInfoRoot;
	static IDirect3DTexture9 *LoadTexture(const std::string &FileName,D3DXIMAGE_INFO *SrcInfo);
public:
	cTextureManager();
	~cTextureManager();
	OWNERSHIP cTexture *GetTextureByName(const std::string &TextureName);
	cTexture *GetTextureByFileName(const std::string &TextureFileName);
	void NoLongerReferenced(cTexture *Texture);
};
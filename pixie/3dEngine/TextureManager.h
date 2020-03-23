#pragma once

class cTextureManager: public tSingleton<cTextureManager>
{
	cDevice *mDevice;
	typedef std::map<CString,cTexture *> cTextureFileMap;
	cTextureFileMap mTextureFileMap; // filename -> texture
	typedef std::map<CString,cTexture *> cTextureMap;
	cTextureMap mTextureMap;
	Expression::cObject *mTextureInfoRoot;
	static IDirect3DTexture9 *LoadTexture(const CString &FileName,D3DXIMAGE_INFO *SrcInfo);
public:
	cTextureManager();
	~cTextureManager();
	OWNERSHIP cTexture *GetTextureByName(const CString &TextureName);
	cTexture *GetTextureByFileName(const CString &TextureFileName);
	void NoLongerReferenced(cTexture *Texture);
};
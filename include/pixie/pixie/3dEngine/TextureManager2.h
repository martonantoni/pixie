#pragma once

class cTextureManager2
{
	struct cImageFile
	{
		tIntrusivePtr<cTexture> mTexture;
		std::string mPath;
		cPoint mSize;
	};
	struct cTileSetData
	{
		cImageFile *mImageFile=nullptr;
		cPoint mTileSize;
		float mTileWidthRatio, mTileHeightRatio; // tile dimensions to entire image ratio. the entire image is 1.0
	};
	struct cTextureData
	{
		cImageFile *mImageFile=nullptr;
		tIntrusivePtr<cTexture> mTexture;
		cTextureInfo mTextureInfo;
		cTextureData(const cTextureInfo &TextureInfo): mTextureInfo(TextureInfo) {}
	};
	typedef std::unordered_map<std::string, cTileSetData *> cTileSetMap;
	cTileSetMap mTileSets;
	typedef std::unordered_map<std::string, std::unique_ptr<cTextureData>> cTextureMap;
	cTextureMap mTextures;
	typedef std::vector<cImageFile *> cImageFiles;
	cImageFiles mImageFiles;
	std::unordered_map<std::string, std::string> mTextureAliases;
	bool AddEntire(const std::string &Name, cImageFile *ImageFile);
	void ProcessInfoFile(const std::string &Path, const cPath &TextureFilePath);
	static IDirect3DTexture9 *LoadTexture(const cPath &FileName, D3DXIMAGE_INFO *SrcInfo);
public:
	cTextureManager2()=default;
	~cTextureManager2()=default;
	void Initialize();

	tIntrusivePtr<cTexture> loadFromFile(const std::filesystem::path &path);

	tIntrusivePtr<cTexture> GetTexture(const std::string &TextureName, bool IsOptional=false) const;
	tIntrusivePtr<cTexture> GetTileTexture(const std::string &TileSetName, int TileX, int TileY) const;
	tIntrusivePtr<cTexture> GetTileSetTexture(const std::string &TileSetName) const; // when you want to get the entire texture of a tileset
	cPoint GetTileSetSize(const std::string &TileSetName) const;

	static tIntrusivePtr<cTexture> LoadFromMemory(const void* memory, size_t size);
};

extern cTextureManager2 theTextureManager;
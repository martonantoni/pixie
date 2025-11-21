#include "StdAfx.h"
#include "pixie/pixie/i_pixie.h"
// info file format:
// ~~~~~~~~~~~~~~~~~
// 
// lines starting with ';' or '#' are ignored
//
//
// command lines:
//
// entire <texture name>
//
// part <texture name> <x> <y> <w> <h>    a negative value means: entire width/height plus the value
//                                        for example: -16 0 16 64  means the right most 16 pixels
//                                        '*' means the entire width/height
//                                        for example: 0 0 * 16  means the top 16 pixels of the image (entire width)
//
// tileset <tileset_name> <tile_witdh> <tile_height> [
// tile <texture_name> <tile_coord_x> <tile_coord_y>     this line is invalid if there was no tileset command yet
//
// ignore_rest                            everything bellow this line is ignored

IDirect3DTexture9 *cTextureManager2::LoadTexture(const cPath &FileName,D3DXIMAGE_INFO *SrcInfo)
{
	IDirect3DTexture9 *d3dTexture;

	//Use a magenta colourkey
	D3DCOLOR colorkey = 0;//  0xFFFFffFF;

	// Load image from file
	
	if (FAILED(D3DXCreateTextureFromFileEx (*theDevice, FileName.c_str(),
		D3DX_DEFAULT_NONPOW2 , D3DX_DEFAULT_NONPOW2 ,

//	D3DX_DEFAULT, 0,

		//		0, 0, 
		D3DX_DEFAULT,  // <---- mip levels
//		1, 
		0, // <--- usage
		D3DFMT_A8R8G8B8,
		D3DPOOL_MANAGED, 
//		D3DX_FILTER_NONE, 
		D3DX_DEFAULT, // <---- filtering
		D3DX_DEFAULT, 
		colorkey, SrcInfo, NULL, &d3dTexture)))
	{
		int Err=GetLastError();
		MainLog->Log("Failed to load texture file: %s, Error: %d", FileName.c_str(), Err);
		return NULL;
	}

	//Return the newly made texture
	return d3dTexture;
}

tIntrusivePtr<cTexture> cTextureManager2::loadFromFile(const std::filesystem::path& path)
{
	D3DXIMAGE_INFO SourceInfo;
	IDirect3DTexture9* Direct3DTexture = LoadTexture(path, &SourceInfo);
	if(!Direct3DTexture)
    {
        return{};
    }
	auto texture = make_intrusive_ptr<cTexture>(Direct3DTexture, SourceInfo.Width, SourceInfo.Height);
	Direct3DTexture->Release();
	return texture;
}

tIntrusivePtr<cTexture> cTextureManager2::LoadFromMemory(const void* memory, size_t size)
{
	IDirect3DTexture9 *d3dTexture;
	D3DXIMAGE_INFO SourceInfo;

	//Use a magenta colourkey
	D3DCOLOR colorkey = 0;//  0xFFFFffFF;

						  // Load image from file
	if(FAILED(D3DXCreateTextureFromFileInMemoryEx(*theDevice, memory, (UINT)size,
		D3DX_DEFAULT_NONPOW2, D3DX_DEFAULT_NONPOW2,

		D3DX_DEFAULT,  // <---- mip levels
					   //		1, 
		0, // <--- usage
		D3DFMT_A8R8G8B8,
		D3DPOOL_MANAGED,
		//		D3DX_FILTER_NONE, 
		D3DX_DEFAULT, // <---- filtering
		D3DX_DEFAULT,
		colorkey, &SourceInfo, NULL, &d3dTexture)))
	{
		int Err = GetLastError();
		MainLog->Log("Failed to load texture from memory -- Error: %d", Err);
		return{};
	}
	auto texture= make_intrusive_ptr<cTexture>(d3dTexture, SourceInfo.Width, SourceInfo.Height);
	d3dTexture->Release();

	return std::move(texture);
}


bool cTextureManager2::AddEntire(const std::string &Name, cImageFile *ImageFile)
{
	auto &TextureData=mTextures[Name];
	if(TextureData)
	{
		MainLog->Log("Warning! Duplicated texture (\"%s\") reference.",Name.c_str());
		return false;
	}
	TextureData=std::make_unique<cTextureData>(cTextureInfo(ImageFile->mSize));
	TextureData->mImageFile=ImageFile;
	TextureData->mTexture=ImageFile->mTexture;
	return true;
}

#define HANDLE_INVALID_LINE(Condition) \
	if(Condition) \
	{ \
		ASSERT(false); \
		MainLog->Log("Warning! Invalid line \"%.*s\" in %s (condition: %s)",Line.size(),Line.data(),Path.c_str(),#Condition); \
		continue; \
	}

void cTextureManager2::ProcessInfoFile(const std::string &Path, const cPath &TextureFilePath)
{
	D3DXIMAGE_INFO SourceInfo;
	IDirect3DTexture9 *Direct3DTexture=LoadTexture(TextureFilePath,&SourceInfo);
	if(Direct3DTexture)
	{
		cImageFile *ImageFile=new cImageFile;
		ImageFile->mTexture=make_intrusive_ptr<cTexture>(Direct3DTexture,SourceInfo.Width,SourceInfo.Height);
		Direct3DTexture->Release();
		ImageFile->mSize.x=SourceInfo.Width;
		ImageFile->mSize.y=SourceInfo.Height;
		ImageFile->mPath=Path;
		mImageFiles.push_back(ImageFile);

		cTileSetData *CurrentTileSet=NULL;
		if(Path.empty())
		{
			AddEntire(TextureFilePath.GetFileNameWithoutExtension(), ImageFile);
			return;
		}
		cFastFileReader InfoFile(Path);
		for(;;)
		{
			auto [Line, isEOF] = InfoFile.getNextLine();
			if(isEOF)
				break;
			if(Line.size()==0)
				continue;
			if(Line[0]==';'||Line[0]=='#')
				continue;
			cStringVector LineTokens((std::string)Line," \t",false);
			if(LineTokens.empty())
				continue;
			if(LineTokens[0]=="entire")
			{
				HANDLE_INVALID_LINE(LineTokens.size()!=2);
				AddEntire(LineTokens[1], ImageFile);
			}
			else if(LineTokens[0]=="tileset")
			{
				HANDLE_INVALID_LINE(LineTokens.size()!=4);
				cTileSetData *&TileSetData=mTileSets[LineTokens[1]];
				if(TileSetData)
				{
					MainLog->Log("Warning! Duplicated tileset (\"%s\") reference. Found in these files: %s and %s",
						LineTokens[1].c_str(),TileSetData->mImageFile->mPath.c_str(),Path.c_str());
					continue;
				}
				CurrentTileSet=TileSetData=new cTileSetData;
				TileSetData->mImageFile=ImageFile;
				TileSetData->mTileSize.x=atoi(LineTokens[2].c_str());
				TileSetData->mTileSize.y=atoi(LineTokens[3].c_str());
				TileSetData->mTileWidthRatio=float(TileSetData->mTileSize.x)/float(SourceInfo.Width);
				TileSetData->mTileHeightRatio=float(TileSetData->mTileSize.y)/float(SourceInfo.Height);
			}
			else if(LineTokens[0]=="tile")
			{
				HANDLE_INVALID_LINE(CurrentTileSet==NULL);
				HANDLE_INVALID_LINE(LineTokens.size()!=4);
				auto &TextureData=mTextures[LineTokens[1]];
				if(TextureData)
				{
					MainLog->Log("Warning! Duplicated texture (\"%s\") reference. Found in these files: %s and %s",
						LineTokens[1].c_str(),TextureData->mImageFile->mPath.c_str(),Path.c_str());
					continue;
				}
				int TileX=atoi(LineTokens[2].c_str());
				int TileY=atoi(LineTokens[3].c_str());
				TextureData=std::make_unique<cTextureData>(cTextureInfo(
					cRect(TileX*CurrentTileSet->mTileSize.x, TileY*CurrentTileSet->mTileSize.y,
						CurrentTileSet->mTileSize.x, CurrentTileSet->mTileSize.y),
						CurrentTileSet->mImageFile->mSize));
				TextureData->mImageFile=ImageFile;
				TextureData->mTexture=make_intrusive_ptr<cTexture>(*ImageFile->mTexture,TextureData->mTextureInfo);
			}
			else if(LineTokens[0]=="part")
			{
				HANDLE_INVALID_LINE(LineTokens.size()!=6);
				int PixelX=atoi(LineTokens[2].c_str());
				int PixelY=atoi(LineTokens[3].c_str());
				int PixelW=LineTokens[4]=="*"?ImageFile->mSize.x:atoi(LineTokens[4].c_str());
				int PixelH=LineTokens[5]=="*"?ImageFile->mSize.y:atoi(LineTokens[5].c_str());
				if(PixelX<0)
					PixelX=ImageFile->mSize.x+PixelX;
				if(PixelY<0)
					PixelY=ImageFile->mSize.y+PixelY;
				if(PixelW<0)
					PixelW=ImageFile->mSize.x+PixelW;
				if(PixelH<0)
					PixelH=ImageFile->mSize.y+PixelH;
				HANDLE_INVALID_LINE(!(PixelX<ImageFile->mSize.x));
				HANDLE_INVALID_LINE(!(PixelY<ImageFile->mSize.y));
				HANDLE_INVALID_LINE(!(PixelX+PixelW<=ImageFile->mSize.x));
				HANDLE_INVALID_LINE(!(PixelY+PixelH<=ImageFile->mSize.y));
				auto &TextureData=mTextures[LineTokens[1]];
				if(TextureData)
				{
					MainLog->Log("Warning! Duplicated texture (\"%s\") reference. Found in these files: %s and %s",
						LineTokens[1].c_str(),TextureData->mImageFile->mPath.c_str(),Path.c_str());
					continue;
				}
				TextureData=std::make_unique<cTextureData>(cTextureInfo(cRect(PixelX, PixelY, PixelW, PixelH), ImageFile->mSize));
				ASSERT(LineTokens[1]!="archer");
				TextureData->mTexture=make_intrusive_ptr<cTexture>(*ImageFile->mTexture,TextureData->mTextureInfo);	
			}
			else if(LineTokens[0]=="ignore_rest")
			{
				break;
			}
			else
			{
				MainLog->Log("Warning! Invalid command \"%s\" in %s",LineTokens[0].c_str(),Path.c_str());
				ASSERT(false);
			}
		}
	}
}

#undef HANDLE_INVALID_LINE


void cTextureManager2::Initialize()
{
// 	std::shared_ptr<cConfig> Config=cLuaBasedConfig::CreateConfig("pixie_system.folders", theLuaState);
// 	std::string TexturesFolderConfig=Config->get<std::string>("textures", "textures"s);
	std::string TexturesFolderConfig = theGlobalConfig->get<std::string>("pixie_system.folders.textures", "textures"s);
	cStringVector TexturesFolders(TexturesFolderConfig, ", ", false);
	for(auto &TexturesFolder: TexturesFolders)
	{
		WIN32_FIND_DATA FindData;
		memset(&FindData, 0, sizeof(WIN32_FIND_DATA));
		HANDLE FindHandle=::FindFirstFile(fmt::sprintf("%s/*.*", TexturesFolder.c_str()).c_str(), &FindData);
		struct cFileNameData
		{
			bool mHasInfoFile=false;
			std::string mExtension;
		};
		std::map<std::string, cFileNameData> Files;
		if(FindHandle!=INVALID_HANDLE_VALUE)
		{
			do
			{
				cPath Path(std::string(FindData.cFileName));
				if(Path.GetExtension()=="info")
				{
					Files[Path.GetFileNameWithoutExtension()].mHasInfoFile=true;
				}
				else
				{
					auto extension = Path.GetExtension();
					std::transform(ALL(extension), extension.begin(), ::tolower);
					if(extension=="dds"||extension=="png"||extension=="jpg")
					{
						auto &FileData=Files[Path.GetFileNameWithoutExtension()];
						if(ASSERTFALSE(!FileData.mExtension.empty()))
						{
							MainLog->Log("Warning: more than one extension for file \"%s\"", Path.GetFileNameWithoutExtension().c_str());
						}
						FileData.mExtension=Path.GetExtension();
					}
				}
			} while(::FindNextFile(FindHandle, &FindData));
			::FindClose(FindHandle);
		}
		for(auto &FileData: Files)
		{
			if(FileData.second.mHasInfoFile)
			{
				ProcessInfoFile(fmt::sprintf("%s/%s.info", TexturesFolder.c_str(), FileData.first.c_str())
					, fmt::sprintf("%s/%s.%s", TexturesFolder.c_str(), FileData.first.c_str(), FileData.second.mExtension.c_str()));
			}
			else
			{
				ProcessInfoFile({}, fmt::sprintf("%s/%s.%s", TexturesFolder.c_str(), FileData.first.c_str(), FileData.second.mExtension.c_str()));
			}
		}
	}
// init aliases:
	theGlobalConfig->createSubConfig("texture_aliases")->forEachString(
		[this](const std::string &Key, const std::string &Value)
    {
        mTextureAliases[Key]=Value;
    });
}

tIntrusivePtr<cTexture> cTextureManager2::GetTexture(const std::string &TextureName, bool IsOptional) const
{
	cTextureMap::const_iterator i=mTextures.find(TextureName);
	if(i!=mTextures.end())
	{
		return i->second->mTexture;
	}
	auto AliasIt=mTextureAliases.find(TextureName);
	if(AliasIt!=mTextureAliases.end())
		return GetTexture(AliasIt->second);
	if(!IsOptional)
	{
		MainLog->Log("Error! Texture not found: %s", TextureName.c_str());
		ASSERT(false);
	}
	return nullptr;
}

tIntrusivePtr<cTexture> cTextureManager2::GetTileTexture(const std::string &TileSetName,int TileX,int TileY) const
{
	auto i=mTileSets.find(TileSetName);
	if(i!=mTileSets.end())
	{
		cTileSetData *TileSetData=i->second;
		return make_intrusive_ptr<cTexture>(*TileSetData->mImageFile->mTexture,
			cTextureInfo(cRect(TileX*TileSetData->mTileSize.x, TileY*TileSetData->mTileSize.y,
				TileSetData->mTileSize.x, TileSetData->mTileSize.y), TileSetData->mImageFile->mSize));
	}
	return nullptr;
}

cPoint cTextureManager2::GetTileSetSize(const std::string &TileSetName) const
{
	auto i=mTileSets.find(TileSetName);
	return i!=mTileSets.end()?i->second->mImageFile->mSize/i->second->mTileSize:cPoint{0, 0};
}

tIntrusivePtr<cTexture> cTextureManager2::GetTileSetTexture(const std::string &TileSetName) const
{
	auto i=mTileSets.find(TileSetName);
	return ASSERTTRUE(i!=mTileSets.end())?i->second->mImageFile->mTexture:nullptr;
}

cTextureManager2 theTextureManager;
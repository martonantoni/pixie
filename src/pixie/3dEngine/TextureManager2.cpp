#include "StdAfx.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

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

ID3D11Texture2D* cTextureManager2::CreateTexture(const unsigned char* Pixels, int Width, int Height)
{
	D3D11_TEXTURE2D_DESC Desc = {};
	Desc.Width = static_cast<UINT>(Width);
	Desc.Height = static_cast<UINT>(Height);
	Desc.MipLevels = 0; // allocate the full mip chain
	Desc.ArraySize = 1;
	Desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; // stb_image gives us RGBA
	Desc.SampleDesc.Count = 1;
	Desc.Usage = D3D11_USAGE_DEFAULT;
	Desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
	Desc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;

	ID3D11Texture2D* Texture = nullptr;
	HRESULT Result = theDevice->GetD3DObject()->CreateTexture2D(&Desc, nullptr, &Texture);
	if (FAILED(Result))
	{
		MainLog->Log("Failed to create Direct3D texture ({}x{}): {}", Width, Height, Pixie_GetErrorCodeText(Result));
		return nullptr;
	}

	// Upload only the largest mip level. Direct3D will generate the rest.
	theDevice->GetDeviceContext()->UpdateSubresource(
		Texture,
		0,
		nullptr,
		Pixels,
		static_cast<UINT>(Width * 4),
		0);

	ID3D11ShaderResourceView* ShaderResourceView = nullptr;
	Result = theDevice->GetD3DObject()->CreateShaderResourceView(
		Texture,
		nullptr,
		&ShaderResourceView);

	if (FAILED(Result))
	{
		MainLog->Log("Failed to create shader resource view for texture ({}x{}): {}", Width, Height, Pixie_GetErrorCodeText(Result));
		Texture->Release();
		return nullptr;
	}

	theDevice->GetDeviceContext()->GenerateMips(ShaderResourceView);
	ShaderResourceView->Release();

	return Texture;
}

ID3D11Texture2D* cTextureManager2::LoadTexture(const cPath& FileName, cPoint* Size)
{
	int Width = 0;
	int Height = 0;
	int Channels = 0;
	unsigned char* Pixels = stbi_load(FileName.c_str(), &Width, &Height, &Channels, STBI_rgb_alpha);
	if (!Pixels)
	{
		MainLog->Log("Failed to load texture file: {} -- {}", FileName.c_str(), stbi_failure_reason());
		return nullptr;
	}

	ID3D11Texture2D* Texture = CreateTexture(Pixels, Width, Height);
	stbi_image_free(Pixels);

	if (Texture && Size)
	{
		Size->x = Width;
		Size->y = Height;
	}
	return Texture;
}

tIntrusivePtr<cTexture> cTextureManager2::loadFromFile(const std::filesystem::path& path)
{
	cPoint Size;
	ID3D11Texture2D* Direct3DTexture = LoadTexture(path, &Size);
	if (!Direct3DTexture)
	{
		return {};
	}

	auto Texture = make_intrusive_ptr<cTexture>(Direct3DTexture, Size.x, Size.y);
	Direct3DTexture->Release();
	return Texture;
}

tIntrusivePtr<cTexture> cTextureManager2::LoadFromMemory(const void* memory, size_t size)
{
	int Width = 0;
	int Height = 0;
	int Channels = 0;
	unsigned char* Pixels = stbi_load_from_memory(
		reinterpret_cast<const stbi_uc*>(memory),
		static_cast<int>(size),
		&Width,
		&Height,
		&Channels,
		STBI_rgb_alpha);

	if (!Pixels)
	{
		MainLog->Log("Failed to load texture from memory -- {}", stbi_failure_reason());
		return {};
	}

	ID3D11Texture2D* Direct3DTexture = CreateTexture(Pixels, Width, Height);
	stbi_image_free(Pixels);

	if (!Direct3DTexture)
	{
		return {};
	}

	auto Texture = make_intrusive_ptr<cTexture>(Direct3DTexture, Width, Height);
	Direct3DTexture->Release();
	return Texture;
}

bool cTextureManager2::AddEntire(const std::string& Name, cImageFile* ImageFile)
{
	auto& TextureData = mTextures[Name];
	if (TextureData)
	{
		MainLog->Log("Warning! Duplicated texture (\"{}\") reference.", Name);
		return false;
	}
	TextureData = std::make_unique<cTextureData>(cTextureInfo(ImageFile->mSize));
	TextureData->mImageFile = ImageFile;
	TextureData->mTexture = ImageFile->mTexture;
	return true;
}

#define HANDLE_INVALID_LINE(Condition) \
	if(Condition) \
	{ \
		ASSERT(false); \
		MainLog->Log("Warning! Invalid line \"{}\" in {} (condition: {})", std::string(line.data(), line.size()), Path, #Condition); \
		continue; \
	}

void cTextureManager2::ProcessInfoFile(const std::string& Path, const cPath& TextureFilePath)
{
	cPoint SourceSize;
	ID3D11Texture2D* Direct3DTexture = LoadTexture(TextureFilePath, &SourceSize);
	if (Direct3DTexture)
	{
		cImageFile* ImageFile = new cImageFile;
		ImageFile->mTexture = make_intrusive_ptr<cTexture>(Direct3DTexture, SourceSize.x, SourceSize.y);
		Direct3DTexture->Release();
		ImageFile->mSize.x = SourceSize.x;
		ImageFile->mSize.y = SourceSize.y;
		ImageFile->mPath = Path;
		mImageFiles.push_back(ImageFile);

		cTileSetData* CurrentTileSet = NULL;
		if (Path.empty())
		{
			AddEntire(TextureFilePath.GetFileNameWithoutExtension(), ImageFile);
			return;
		}
		cFastFileReader InfoFile(Path);
		for (auto line : InfoFile)
		{
			if (line.empty())
				continue;
			if (line[0] == ';' || line[0] == '#')
				continue;
			cStringVector LineTokens(line, " \t", false);
			if (LineTokens.empty())
				continue;
			if (LineTokens[0] == "entire")
			{
				HANDLE_INVALID_LINE(LineTokens.size() != 2);
				AddEntire(LineTokens[1], ImageFile);
			}
			else if (LineTokens[0] == "tileset")
			{
				HANDLE_INVALID_LINE(LineTokens.size() != 4);
				cTileSetData*& TileSetData = mTileSets[LineTokens[1]];
				if (TileSetData)
				{
					MainLog->Log("Warning! Duplicated tileset (\"{}\") reference. Found in these files: {} and {}", LineTokens[1], TileSetData->mImageFile->mPath, Path);
					continue;
				}
				CurrentTileSet = TileSetData = new cTileSetData;
				TileSetData->mImageFile = ImageFile;
				TileSetData->mTileSize.x = atoi(LineTokens[2].c_str());
				TileSetData->mTileSize.y = atoi(LineTokens[3].c_str());
				TileSetData->mTileWidthRatio = float(TileSetData->mTileSize.x) / float(SourceSize.x);
				TileSetData->mTileHeightRatio = float(TileSetData->mTileSize.y) / float(SourceSize.y);
			}
			else if (LineTokens[0] == "tile")
			{
				HANDLE_INVALID_LINE(CurrentTileSet == NULL);
				HANDLE_INVALID_LINE(LineTokens.size() != 4);
				auto& TextureData = mTextures[LineTokens[1]];
				if (TextureData)
				{
					MainLog->Log("Warning! Duplicated texture (\"{}\") reference. Found in these files: {} and {}", LineTokens[1], TextureData->mImageFile->mPath, Path);
					continue;
				}
				int TileX = atoi(LineTokens[2].c_str());
				int TileY = atoi(LineTokens[3].c_str());
				TextureData = std::make_unique<cTextureData>(cTextureInfo(
					cRect(TileX * CurrentTileSet->mTileSize.x, TileY * CurrentTileSet->mTileSize.y,
						CurrentTileSet->mTileSize.x, CurrentTileSet->mTileSize.y),
					CurrentTileSet->mImageFile->mSize));
				TextureData->mImageFile = ImageFile;
				TextureData->mTexture = make_intrusive_ptr<cTexture>(*ImageFile->mTexture, TextureData->mTextureInfo);
			}
			else if (LineTokens[0] == "part")
			{
				HANDLE_INVALID_LINE(LineTokens.size() != 6);
				int PixelX = atoi(LineTokens[2].c_str());
				int PixelY = atoi(LineTokens[3].c_str());
				int PixelW = LineTokens[4] == "*" ? ImageFile->mSize.x : atoi(LineTokens[4].c_str());
				int PixelH = LineTokens[5] == "*" ? ImageFile->mSize.y : atoi(LineTokens[5].c_str());
				if (PixelX < 0)
					PixelX = ImageFile->mSize.x + PixelX;
				if (PixelY < 0)
					PixelY = ImageFile->mSize.y + PixelY;
				if (PixelW < 0)
					PixelW = ImageFile->mSize.x + PixelW;
				if (PixelH < 0)
					PixelH = ImageFile->mSize.y + PixelH;
				HANDLE_INVALID_LINE(!(PixelX < ImageFile->mSize.x));
				HANDLE_INVALID_LINE(!(PixelY < ImageFile->mSize.y));
				HANDLE_INVALID_LINE(!(PixelX + PixelW <= ImageFile->mSize.x));
				HANDLE_INVALID_LINE(!(PixelY + PixelH <= ImageFile->mSize.y));
				auto& TextureData = mTextures[LineTokens[1]];
				if (TextureData)
				{
					MainLog->Log("Warning! Duplicated texture (\"{}\") reference. Found in these files: {} and {}", LineTokens[1], TextureData->mImageFile->mPath, Path);
					continue;
				}
				TextureData = std::make_unique<cTextureData>(cTextureInfo(cRect(PixelX, PixelY, PixelW, PixelH), ImageFile->mSize));
				ASSERT(LineTokens[1] != "archer");
				TextureData->mTexture = make_intrusive_ptr<cTexture>(*ImageFile->mTexture, TextureData->mTextureInfo);
			}
			else if (LineTokens[0] == "ignore_rest")
			{
				break;
			}
			else
			{
				MainLog->Log("Warning! Invalid command \"{}\" in {}", LineTokens[0], Path);
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
	for (auto& TexturesFolder : TexturesFolders)
	{
		WIN32_FIND_DATA FindData;
		memset(&FindData, 0, sizeof(WIN32_FIND_DATA));
		HANDLE FindHandle = ::FindFirstFile(std::format("{}/*.*", TexturesFolder).c_str(), &FindData);
		struct cFileNameData
		{
			bool mHasInfoFile = false;
			std::string mExtension;
		};
		std::map<std::string, cFileNameData> Files;
		if (FindHandle != INVALID_HANDLE_VALUE)
		{
			do
			{
				cPath Path(std::string(FindData.cFileName));
				if (Path.GetExtension() == "info")
				{
					Files[Path.GetFileNameWithoutExtension()].mHasInfoFile = true;
				}
				else
				{
					auto extension = Path.GetExtension();
					std::transform(ALL(extension), extension.begin(), ::tolower);
					if (extension == "dds" || extension == "png" || extension == "jpg")
					{
						auto& FileData = Files[Path.GetFileNameWithoutExtension()];
						if (ASSERTFALSE(!FileData.mExtension.empty()))
						{
							MainLog->Log("Warning: more than one extension for file \"{}\"", Path.GetFileNameWithoutExtension());
						}
						FileData.mExtension = Path.GetExtension();
					}
				}
			} while (::FindNextFile(FindHandle, &FindData));
			::FindClose(FindHandle);
		}
		for (auto& FileData : Files)
		{
			if (FileData.second.mHasInfoFile)
			{
				ProcessInfoFile(std::format("{}/{}.info", TexturesFolder, FileData.first)
					, std::format("{}/{}.{}", TexturesFolder, FileData.first, FileData.second.mExtension));
			}
			else
			{
				ProcessInfoFile({}, std::format("{}/{}.{}", TexturesFolder, FileData.first, FileData.second.mExtension));
			}
		}
	}
	// init aliases:
	theGlobalConfig->createSubConfig("texture_aliases")->forEachString(
		[this](const std::string& Key, const std::string& Value)
		{
			mTextureAliases[Key] = Value;
		});
}

tIntrusivePtr<cTexture> cTextureManager2::GetTexture(const std::string& TextureName, bool IsOptional) const
{
	cTextureMap::const_iterator i = mTextures.find(TextureName);
	if (i != mTextures.end())
	{
		return i->second->mTexture;
	}
	auto AliasIt = mTextureAliases.find(TextureName);
	if (AliasIt != mTextureAliases.end())
		return GetTexture(AliasIt->second);
	if (!IsOptional)
	{
		MainLog->Log("Error! Texture not found: {}", TextureName);
		ASSERT(false);
	}
	return nullptr;
}

tIntrusivePtr<cTexture> cTextureManager2::GetTileTexture(const std::string& TileSetName, int TileX, int TileY) const
{
	auto i = mTileSets.find(TileSetName);
	if (i != mTileSets.end())
	{
		cTileSetData* TileSetData = i->second;
		return make_intrusive_ptr<cTexture>(*TileSetData->mImageFile->mTexture,
			cTextureInfo(cRect(TileX * TileSetData->mTileSize.x, TileY * TileSetData->mTileSize.y,
				TileSetData->mTileSize.x, TileSetData->mTileSize.y), TileSetData->mImageFile->mSize));
	}
	return nullptr;
}

cPoint cTextureManager2::GetTileSetSize(const std::string& TileSetName) const
{
	auto i = mTileSets.find(TileSetName);
	return i != mTileSets.end() ? i->second->mImageFile->mSize / i->second->mTileSize : cPoint{ 0, 0 };
}

tIntrusivePtr<cTexture> cTextureManager2::GetTileSetTexture(const std::string& TileSetName) const
{
	auto i = mTileSets.find(TileSetName);
	return ASSERTTRUE(i != mTileSets.end()) ? i->second->mImageFile->mTexture : nullptr;
}

cTextureManager2 theTextureManager;
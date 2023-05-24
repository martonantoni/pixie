#include "StdAfx.h"
#include "pixie/pixie/i_pixie.h"

cEffectHolder::cEffectHolder(const std::string &EffectName)
{
	std::string FileName=fmt::sprintf("%s.fxo",EffectName);
	RELEASE_ASSERT_EXT(::DoesFileExist(FileName),fmt::sprintf("Effect file %s is missing",FileName));
	ID3DXBuffer *ErrorBuffer=NULL;
	if(D3DXCreateEffectFromFile(*cDevice::Get(),FileName.c_str(),NULL,NULL,D3DXFX_NOT_CLONEABLE,NULL,&Effect,&ErrorBuffer)!=D3D_OK)
	{
		RELEASE_ASSERT_EXT(false,fmt::sprintf("Loading effect %s failed.\nError: %s",EffectName,ErrorBuffer->GetBufferPointer()));
	}
}

cEffectHolder::~cEffectHolder()
{
}


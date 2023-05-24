#pragma once

#define D3V(Function) \
do \
{ \
	/*MainLog->Log("Calling %s",#Function);*/ \
	/*MainLog->Flush();*/ \
	/*OutputDebugString(#Function "\n");*/ \
	HRESULT Result=Function;\
	/*OutputDebugString("Done\n");*/ \
	/*MainLog->Log("result: %d",Result);*/ \
	RELEASE_ASSERT_EXT(Result==D3D_OK,fmt::sprintf("Function call \"%s\" failed.\nError code: %s",#Function,Pixie_GetErrorCodeText(Result)));\
} while(false)

std::string Pixie_GetErrorCodeText(HRESULT Result);

inline void StopOnError(HRESULT Result)
{
	RELEASE_ASSERT_EXT(Result==D3D_OK,fmt::sprintf("D3D error code: %s",Pixie_GetErrorCodeText(Result)));
}

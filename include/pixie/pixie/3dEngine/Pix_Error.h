#pragma once

#define D3V(Function) \
do \
{ \
    HRESULT Result = Function; \
    RELEASE_ASSERT_EXT(SUCCEEDED(Result), fmt::sprintf("Function call \"%s\" failed.\nError code: %s", #Function, Pixie_GetErrorCodeText(Result))); \
} while(false)

std::string Pixie_GetErrorCodeText(HRESULT Result);

inline void StopOnError(HRESULT Result)
{
    RELEASE_ASSERT_EXT(SUCCEEDED(Result), fmt::sprintf("DirectX error code: %s", Pixie_GetErrorCodeText(Result)));
}

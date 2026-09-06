#pragma once

#define D3V(Function) \
do \
{ \
    HRESULT Result = Function; \
    RELEASE_ASSERT_EXT(SUCCEEDED(Result), format("Function call \"{}\" failed.\nError code: {}", #Function, Pixie_GetErrorCodeText(Result))); \
} while(false)

std::string Pixie_GetErrorCodeText(HRESULT Result);

inline void StopOnError(HRESULT Result)
{
    RELEASE_ASSERT_EXT(SUCCEEDED(Result), format("DirectX error code: {}", Pixie_GetErrorCodeText(Result)));
}

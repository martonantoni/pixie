#include "ps_input"

cbuffer SpriteConstants : register(b0)
{
    float2 TargetSize;
    float2 Padding;
};

struct VSInput
{
    float2 ScreenPos : POSITION; // Screen position in pixels
    uint Color : COLOR;

    float2 TexCoord0 : TEXCOORD0;
    float2 TexCoord1 : TEXCOORD1;
    float2 TexCoord2 : TEXCOORD2;
    float2 TexCoord3 : TEXCOORD3;

    float4 PixelPos : TEXCOORD4; // Position within the sprite in pixels from each border (left, top, right, bottom)
    float4 Parameters : PARAM;
};

float4 UnpackARGB(uint color)
{
    return float4(
        ((color >> 16) & 255) / 255.0f,
        ((color >> 8) & 255) / 255.0f,
        (color & 255) / 255.0f,
        ((color >> 24) & 255) / 255.0f);
}

VSOutput VSMain(VSInput input)
{
    VSOutput output;

    float2 clipPosition;
    clipPosition.x = input.ScreenPos.x * (2.0f / TargetSize.x) - 1.0f;
    clipPosition.y = 1.0f - input.ScreenPos.y * (2.0f / TargetSize.y);

    output.screenPosition = float4(clipPosition, 0.5f, 1.0f);
    output.color = UnpackARGB(input.Color);

    output.texCoord = input.TexCoord0;
    output.texCoord1 = input.TexCoord1;
    output.texCoord2 = input.TexCoord2;
    output.texCoord3 = input.TexCoord3;

    output.params = input.Parameters;
    output.pixelPos = input.PixelPos;

    return output;
}
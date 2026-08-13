cbuffer SpriteConstants : register(b0)
{
    float2 TargetSize;
    float2 Padding;
};

Texture2D SpriteTexture : register(t0);
SamplerState SpriteSampler : register(s0);

struct VSInput
{
    float3 Position : POSITION;
    uint Color : COLOR;
    float2 TexCoord : TEXCOORD0;
};

struct VSOutput
{
    float4 Position : SV_POSITION;
    float4 Color : COLOR;
    float2 TexCoord : TEXCOORD0;
};

float4 UnpackARGB(uint color)
{
    return float4(
        ((color >> 16) & 255) / 255.0f,
        ((color >> 8) & 255) / 255.0f,
        (color & 255) / 255.0f,
        ((color >> 24) & 255) / 255.0f);
}

float4 PSMain(VSOutput input) : SV_TARGET
{
    return SpriteTexture.Sample(SpriteSampler, input.TexCoord) * input.Color;
}

cbuffer SpriteConstants : register(b0)
{
    float2 TargetSize;
    float2 Padding;
};

struct VSInput
{
    float3 Position : POSITION;
    uint Color : COLOR;
    float2 TexCoord : TEXCOORD0;
    float4 Parameters : PARAM;
};

struct VSOutput
{
    float4 Position : SV_POSITION;
    float4 Color : COLOR;
    float2 TexCoord : TEXCOORD0;
    float4 Parameters : TEXCOORD1;
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
    clipPosition.x = input.Position.x * (2.0f / TargetSize.x) - 1.0f;
    clipPosition.y = 1.0f - input.Position.y * (2.0f / TargetSize.y);

    output.Position = float4(clipPosition, input.Position.z, 1.0f);
    output.Color = UnpackARGB(input.Color);
    output.TexCoord = input.TexCoord;
	output.Parameters = input.Parameters;

    return output;
}
Texture2D SpriteTexture : register(t0);
SamplerState SpriteSampler : register(s0);

struct PSInput
{
    float4 Position : SV_POSITION;
    float4 Color : COLOR;
    float2 TexCoord : TEXCOORD0;
	float4 Parameters : TEXCOORD1;
};

float4 PSMain(PSInput input) : SV_TARGET
{
    return SpriteTexture.Sample(SpriteSampler, input.TexCoord) * input.Color;
}

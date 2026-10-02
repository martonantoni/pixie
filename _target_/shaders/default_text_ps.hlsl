#include "ps_input"

Texture2D SpriteTexture : register(t0);
SamplerState SpriteSampler : register(s0);

float4 PSMain(PSInput input) : SV_TARGET
{
    float coverage = SpriteTexture.Sample(SpriteSampler, input.texCoord).r;

    float4 color = input.color;
    color.a *= coverage;

    return color;
}
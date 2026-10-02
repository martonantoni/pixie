#include "ps_input"

Texture2D SpriteTexture : register(t0);
SamplerState SpriteSampler : register(s0);

float4 PSMain(PSInput input) : SV_TARGET
{
    return SpriteTexture.Sample(SpriteSampler, input.texCoord) * input.color;
}

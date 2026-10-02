
float3 lerp3(float3 a, float3 b, float3 c, float t)
{
    return lerp(b, t < 0 ? a : c, abs(t));
}

struct PSInput
{
    float4 screenPosition : SV_POSITION;
    float4 color : COLOR0;
    float2 texCoord : TEXCOORD0;
    float2 texCoord1 : TEXCOORD1;
    float2 texCoord2 : TEXCOORD2;
    float2 texCoord3 : TEXCOORD3;
    
    float4 params : PARAM;
    float4 pixelPos : TEXCOORD4; // x = pixels from left
                                 // y = pixels from top
                                 // z = pixels from right
                                 // w = pixels from bottom
};

typedef PSInput VSOutput;
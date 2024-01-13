
Texture2D<float4> tex : register(t0, space1);
SamplerState wrapPoint : register(s0, space4);
SamplerState wrapLinj : register(s1, space4);
SamplerState wrapAniso : register(s2, space4);

struct PixelInput
{
    float4 ScreenPosition : SV_Position;
    float3 WorldPosition : POSITION0;
    float2 ScreenCoordinate : POSITION1;
    
    float3 Normal : NORMAL;
    float3 Binormal : BINORMAL;
    float3 Tangent : TANGENT;
    
    float2 UV : TEXCOORD;
    float4 Color : COLOR;
};

float4 main(PixelInput anInput) : SV_TARGET
{
    const float3 skyColor = (float3(4, 135, 226) / 255) * 0.8f;
    const float3 sunColor = (float3(226, 82, 4) / 255);
    const float3 ambientColor = float3(0.5f, 0.5f, 0.5f);
    
    const float sunDiffuse = saturate(dot(anInput.Normal, normalize(float3(0, 1, -1))));
    const float skyDiffuse = saturate((dot(anInput.Normal, float3(0, 1, 0)) * 0.5f) + 0.5f);
    const float ambientLight = 0.1f;
    
    const float3 totalLight = saturate((skyColor * skyDiffuse) + (sunColor * sunDiffuse) + (ambientColor * ambientLight));
    
    const float textureRed = tex.Sample(wrapPoint, anInput.UV).r;
    const float textureScalar = clamp(textureRed, 0.4f, 0.9f);
    
    return float4(textureScalar * totalLight * anInput.Color.rgb, 1.0f);
}
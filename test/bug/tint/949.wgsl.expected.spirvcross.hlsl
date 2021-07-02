/* Dumped generated HLSL */
struct lightingInfo
{
    float3 diffuse;
    float3 specular;
};

cbuffer LeftOver : register(b0, space2)
{
    row_major float4x4 _269_u_World : packoffset(c0);
    row_major float4x4 _269_u_ViewProjection : packoffset(c4);
    float _269_u_bumpStrength : packoffset(c8);
    float3 _269_u_cameraPosition : packoffset(c9);
    float _269_u_parallaxScale : packoffset(c9.w);
    float _269_textureInfoName : packoffset(c10);
    float2 _269_tangentSpaceParameter0 : packoffset(c10.z);
};

cbuffer Light0 : register(b0, space0)
{
    float4 light0_vLightData : packoffset(c0);
    float4 light0_vLightDiffuse : packoffset(c1);
    float4 light0_vLightSpecular : packoffset(c2);
    float3 light0_vLightGround : packoffset(c3);
    float4 light0_shadowsInfo : packoffset(c4);
    float2 light0_depthValues : packoffset(c5);
};

Texture2D<float4> TextureSamplerTexture : register(t3, space2);
SamplerState TextureSamplerSampler : register(s2, space2);
Texture2D<float4> TextureSampler1Texture : register(t1, space2);
SamplerState TextureSampler1Sampler : register(s0, space2);
SamplerState bumpSamplerSampler : register(s1, space2);
Texture2D<float4> bumpSamplerTexture : register(t2, space2);

static bool gl_FrontFacing;
static float2 vMainuv;
static float4 v_output1;
static float2 v_uv;
static float4 v_output2;
static float4 glFragColor;

struct SPIRV_Cross_Input
{
    float4 v_output1 : TEXCOORD0;
    float2 vMainuv : TEXCOORD1;
    float4 v_output2 : TEXCOORD2;
    float2 v_uv : TEXCOORD3;
    bool gl_FrontFacing : SV_IsFrontFace;
};

struct SPIRV_Cross_Output
{
    float4 glFragColor : SV_Target0;
};

static float u_Float = 0.0f;
static float3 u_Color = 0.0f.xxx;

float3x3 cotangent_frame(float3 normal, float3 p, float2 uv, float2 tangentSpaceParams)
{
    float3 dp1 = ddx(p);
    float3 dp2 = ddy(p);
    float2 duv1 = ddx(uv);
    float2 duv2 = ddy(uv);
    float3 dp2perp = cross(dp2, normal);
    float3 dp1perp = cross(normal, dp1);
    float3 tangent = (dp2perp * duv1.x) + (dp1perp * duv2.x);
    float3 bitangent = (dp2perp * duv1.y) + (dp1perp * duv2.y);
    tangent *= tangentSpaceParams.x;
    bitangent *= tangentSpaceParams.y;
    float invmax = rsqrt(max(dot(tangent, tangent), dot(bitangent, bitangent)));
    return float3x3(float3(tangent * invmax), float3(bitangent * invmax), float3(normal));
}

float3x3 transposeMat3(float3x3 inMatrix)
{
    float3 i0 = inMatrix[0];
    float3 i1 = inMatrix[1];
    float3 i2 = inMatrix[2];
    float3x3 outMatrix = float3x3(float3(float3(i0.x, i1.x, i2.x)), float3(float3(i0.y, i1.y, i2.y)), float3(float3(i0.z, i1.z, i2.z)));
    return outMatrix;
}

float3 perturbNormalBase(float3x3 cotangentFrame, float3 normal, float scale)
{
    return normalize(mul(normal, cotangentFrame));
}

float3 perturbNormal(float3x3 cotangentFrame, float3 textureSample, float scale)
{
    float3x3 param = cotangentFrame;
    float3 param_1 = (textureSample * 2.0f) - 1.0f.xxx;
    float param_2 = scale;
    return perturbNormalBase(param, param_1, param_2);
}

lightingInfo computeHemisphericLighting(float3 viewDirectionW, float3 vNormal, float4 lightData, float3 diffuseColor, float3 specularColor, float3 groundColor, float glossiness)
{
    float ndl = (dot(vNormal, lightData.xyz) * 0.5f) + 0.5f;
    lightingInfo result = { 0.0f.xxx, 0.0f.xxx };
    result.diffuse = lerp(groundColor, diffuseColor, ndl.xxx);
    float3 angleW = normalize(viewDirectionW + lightData.xyz);
    float specComp = max(0.0f, dot(vNormal, angleW));
    specComp = pow(specComp, max(1.0f, glossiness));
    result.specular = specularColor * specComp;
    return result;
}

void frag_main()
{
    u_Float = 100.0f;
    u_Color = 0.5f.xxx;
    float4 tempTextureRead = TextureSamplerTexture.Sample(TextureSamplerSampler, vMainuv);
    float3 rgb = tempTextureRead.xyz * _269_textureInfoName;
    float3 output5 = normalize(_269_u_cameraPosition - v_output1.xyz);
    float4 output4 = 0.0f.xxxx;
    float2 uvOffset = 0.0f.xx;
    float normalScale = 1.0f / _269_u_bumpStrength;
    float2 _299 = 0.0f.xx;
    if (gl_FrontFacing)
    {
        _299 = v_uv;
    }
    else
    {
        _299 = -v_uv;
    }
    float2 TBNUV = _299;
    float3 param = v_output2.xyz * normalScale;
    float3 param_1 = v_output1.xyz;
    float2 param_2 = TBNUV;
    float2 param_3 = _269_tangentSpaceParameter0;
    float3x3 TBN = cotangent_frame(param, param_1, param_2, param_3);
    float3x3 param_4 = TBN;
    float3x3 invTBN = transposeMat3(param_4);
    float parallaxLimit = length(mul(-output5, invTBN).xy) / mul(-output5, invTBN).z;
    parallaxLimit *= _269_u_parallaxScale;
    float2 vOffsetDir = normalize(mul(-output5, invTBN).xy);
    float2 vMaxOffset = vOffsetDir * parallaxLimit;
    float numSamples = 15.0f + (dot(mul(-output5, invTBN), mul(v_output2.xyz, invTBN)) * (-11.0f));
    float stepSize = 1.0f / numSamples;
    float currRayHeight = 1.0f;
    float2 vCurrOffset = 0.0f.xx;
    float2 vLastOffset = 0.0f.xx;
    float lastSampledHeight = 1.0f;
    float currSampledHeight = 1.0f;
    for (int i = 0; i < 15; i++)
    {
        currSampledHeight = TextureSamplerTexture.Sample(TextureSamplerSampler, v_uv + vCurrOffset).w;
        if (currSampledHeight > currRayHeight)
        {
            float delta1 = currSampledHeight - currRayHeight;
            float delta2 = (currRayHeight + stepSize) - lastSampledHeight;
            float ratio = delta1 / (delta1 + delta2);
            vCurrOffset = (vLastOffset * ratio) + (vCurrOffset * (1.0f - ratio));
            break;
        }
        else
        {
            currRayHeight -= stepSize;
            vLastOffset = vCurrOffset;
            vCurrOffset += (vMaxOffset * stepSize);
            lastSampledHeight = currSampledHeight;
        }
    }
    float2 parallaxOcclusion_0 = vCurrOffset;
    uvOffset = parallaxOcclusion_0;
    float3x3 param_5 = TBN;
    float3 param_6 = TextureSamplerTexture.Sample(TextureSamplerSampler, v_uv + uvOffset).xyz;
    float param_7 = 1.0f / _269_u_bumpStrength;
    float3 _461 = perturbNormal(param_5, param_6, param_7);
    output4 = float4(_461.x, _461.y, _461.z, output4.w);
    float2 output6 = v_uv + uvOffset;
    float4 tempTextureRead1 = TextureSampler1Texture.Sample(TextureSampler1Sampler, output6);
    float3 rgb1 = tempTextureRead1.xyz;
    float3 viewDirectionW = normalize(_269_u_cameraPosition - v_output1.xyz);
    float shadow = 1.0f;
    float glossiness = 1.0f * u_Float;
    float3 diffuseBase = 0.0f.xxx;
    float3 specularBase = 0.0f.xxx;
    float3 normalW = output4.xyz;
    float3 param_8 = viewDirectionW;
    float3 param_9 = normalW;
    float4 param_10 = light0_vLightData;
    float3 param_11 = light0_vLightDiffuse.xyz;
    float3 param_12 = light0_vLightSpecular.xyz;
    float3 param_13 = light0_vLightGround;
    float param_14 = glossiness;
    lightingInfo info = computeHemisphericLighting(param_8, param_9, param_10, param_11, param_12, param_13, param_14);
    shadow = 1.0f;
    diffuseBase += (info.diffuse * shadow);
    specularBase += (info.specular * shadow);
    float3 diffuseOutput = diffuseBase * rgb1;
    float3 specularOutput = specularBase * u_Color;
    float3 output3 = diffuseOutput + specularOutput;
    glFragColor = float4(output3, 1.0f);
}

SPIRV_Cross_Output main(SPIRV_Cross_Input stage_input)
{
    gl_FrontFacing = stage_input.gl_FrontFacing;
    vMainuv = stage_input.vMainuv;
    v_output1 = stage_input.v_output1;
    v_uv = stage_input.v_uv;
    v_output2 = stage_input.v_output2;
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.glFragColor = glFragColor;
    return stage_output;
}
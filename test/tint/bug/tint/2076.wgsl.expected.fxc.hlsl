//
// main
//
struct tint_GammaTransferParams {
  float G;
  float A;
  float B;
  float C;
  float D;
  float E;
  float F;
  uint padding;
};

struct tint_ExternalTextureParams {
  uint numPlanes;
  uint doYuvToRgbConversionOnly;
  float3x4 yuvToRgbConversionMatrix;
  tint_GammaTransferParams gammaDecodeParams;
  tint_GammaTransferParams gammaEncodeParams;
  float3x3 gamutConversionMatrix;
  float3x2 sampleTransform;
  float3x2 loadTransform;
  float2 samplePlane0RectMin;
  float2 samplePlane0RectMax;
  float2 samplePlane1RectMin;
  float2 samplePlane1RectMax;
  uint2 apparentSize;
  float2 plane1CoordFactor;
};


Texture2D<float4> randomTexture_plane0 : register(t1);
Texture2D<float4> randomTexture_plane1 : register(t3);
cbuffer cbuffer_randomTexture_params : register(b4) {
  uint4 randomTexture_params[17];
};
Texture2D<float4> depthTexture : register(t2);
float3x2 v(uint start_byte_offset) {
  uint4 v_1 = randomTexture_params[(start_byte_offset / 16u)];
  float2 v_2 = asfloat((((((start_byte_offset & 15u) >> 2u) == 2u)) ? (v_1.zw) : (v_1.xy)));
  uint4 v_3 = randomTexture_params[((8u + start_byte_offset) / 16u)];
  float2 v_4 = asfloat(((((((8u + start_byte_offset) & 15u) >> 2u) == 2u)) ? (v_3.zw) : (v_3.xy)));
  uint4 v_5 = randomTexture_params[((16u + start_byte_offset) / 16u)];
  return float3x2(v_2, v_4, asfloat(((((((16u + start_byte_offset) & 15u) >> 2u) == 2u)) ? (v_5.zw) : (v_5.xy))));
}

float3x3 v_6(uint start_byte_offset) {
  return float3x3(asfloat(randomTexture_params[(start_byte_offset / 16u)].xyz), asfloat(randomTexture_params[((16u + start_byte_offset) / 16u)].xyz), asfloat(randomTexture_params[((32u + start_byte_offset) / 16u)].xyz));
}

tint_GammaTransferParams v_7(uint start_byte_offset) {
  tint_GammaTransferParams v_8 = {asfloat(randomTexture_params[(start_byte_offset / 16u)][((start_byte_offset & 15u) >> 2u)]), asfloat(randomTexture_params[((4u + start_byte_offset) / 16u)][(((4u + start_byte_offset) & 15u) >> 2u)]), asfloat(randomTexture_params[((8u + start_byte_offset) / 16u)][(((8u + start_byte_offset) & 15u) >> 2u)]), asfloat(randomTexture_params[((12u + start_byte_offset) / 16u)][(((12u + start_byte_offset) & 15u) >> 2u)]), asfloat(randomTexture_params[((16u + start_byte_offset) / 16u)][(((16u + start_byte_offset) & 15u) >> 2u)]), asfloat(randomTexture_params[((20u + start_byte_offset) / 16u)][(((20u + start_byte_offset) & 15u) >> 2u)]), asfloat(randomTexture_params[((24u + start_byte_offset) / 16u)][(((24u + start_byte_offset) & 15u) >> 2u)]), randomTexture_params[((28u + start_byte_offset) / 16u)][(((28u + start_byte_offset) & 15u) >> 2u)]};
  return v_8;
}

float3x4 v_9(uint start_byte_offset) {
  return float3x4(asfloat(randomTexture_params[(start_byte_offset / 16u)]), asfloat(randomTexture_params[((16u + start_byte_offset) / 16u)]), asfloat(randomTexture_params[((32u + start_byte_offset) / 16u)]));
}

tint_ExternalTextureParams v_10(uint start_byte_offset) {
  uint v_11 = randomTexture_params[(start_byte_offset / 16u)][((start_byte_offset & 15u) >> 2u)];
  uint v_12 = randomTexture_params[((4u + start_byte_offset) / 16u)][(((4u + start_byte_offset) & 15u) >> 2u)];
  float3x4 v_13 = v_9((16u + start_byte_offset));
  tint_GammaTransferParams v_14 = v_7((64u + start_byte_offset));
  tint_GammaTransferParams v_15 = v_7((96u + start_byte_offset));
  float3x3 v_16 = v_6((128u + start_byte_offset));
  float3x2 v_17 = v((176u + start_byte_offset));
  float3x2 v_18 = v((200u + start_byte_offset));
  uint4 v_19 = randomTexture_params[((224u + start_byte_offset) / 16u)];
  float2 v_20 = asfloat(((((((224u + start_byte_offset) & 15u) >> 2u) == 2u)) ? (v_19.zw) : (v_19.xy)));
  uint4 v_21 = randomTexture_params[((232u + start_byte_offset) / 16u)];
  float2 v_22 = asfloat(((((((232u + start_byte_offset) & 15u) >> 2u) == 2u)) ? (v_21.zw) : (v_21.xy)));
  uint4 v_23 = randomTexture_params[((240u + start_byte_offset) / 16u)];
  float2 v_24 = asfloat(((((((240u + start_byte_offset) & 15u) >> 2u) == 2u)) ? (v_23.zw) : (v_23.xy)));
  uint4 v_25 = randomTexture_params[((248u + start_byte_offset) / 16u)];
  float2 v_26 = asfloat(((((((248u + start_byte_offset) & 15u) >> 2u) == 2u)) ? (v_25.zw) : (v_25.xy)));
  uint4 v_27 = randomTexture_params[((256u + start_byte_offset) / 16u)];
  uint2 v_28 = ((((((256u + start_byte_offset) & 15u) >> 2u) == 2u)) ? (v_27.zw) : (v_27.xy));
  uint4 v_29 = randomTexture_params[((264u + start_byte_offset) / 16u)];
  tint_ExternalTextureParams v_30 = {v_11, v_12, v_13, v_14, v_15, v_16, v_17, v_18, v_20, v_22, v_24, v_26, v_28, asfloat(((((((264u + start_byte_offset) & 15u) >> 2u) == 2u)) ? (v_29.zw) : (v_29.xy)))};
  return v_30;
}

[numthreads(1, 1, 1)]
void main() {
  v_10(0u);
}

//
// main2
//

SamplerState v : register(s1);
Texture2D<float4> depthTexture : register(t2);
[numthreads(1, 1, 1)]
void main2() {
}


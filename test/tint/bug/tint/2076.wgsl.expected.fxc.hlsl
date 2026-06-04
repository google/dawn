//
// main
//
struct tint_TransferFunctionParams {
  uint mode;
  float A;
  float B;
  float C;
  float D;
  float E;
  float F;
  float G;
};

struct tint_ExternalTextureParams {
  uint numPlanes;
  uint doYuvToRgbConversionOnly;
  float3x4 yuvToRgbConversionMatrix;
  tint_TransferFunctionParams srcTransferFunction;
  tint_TransferFunctionParams dstTransferFunction;
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


cbuffer cbuffer_randomTexture_params : register(b4) {
  uint4 randomTexture_params[17];
};
Texture2D<float4> randomTexture_plane0 : register(t1);
Texture2D<float4> randomTexture_plane1 : register(t3);
Texture2D<float4> depthTexture : register(t2);
float3x2 v(uint start_byte_offset) {
  uint4 v_1 = randomTexture_params[(start_byte_offset / 16u)];
  float2 v_2 = asfloat((((((start_byte_offset & 15u) >> 2u) == 2u)) ? (v_1.zw) : (v_1.xy)));
  uint v_3 = (8u + start_byte_offset);
  uint4 v_4 = randomTexture_params[(v_3 / 16u)];
  float2 v_5 = asfloat((((((v_3 & 15u) >> 2u) == 2u)) ? (v_4.zw) : (v_4.xy)));
  uint v_6 = (16u + start_byte_offset);
  uint4 v_7 = randomTexture_params[(v_6 / 16u)];
  return float3x2(v_2, v_5, asfloat((((((v_6 & 15u) >> 2u) == 2u)) ? (v_7.zw) : (v_7.xy))));
}

float3x3 v_8(uint start_byte_offset) {
  return float3x3(asfloat(randomTexture_params[(start_byte_offset / 16u)].xyz), asfloat(randomTexture_params[((16u + start_byte_offset) / 16u)].xyz), asfloat(randomTexture_params[((32u + start_byte_offset) / 16u)].xyz));
}

tint_TransferFunctionParams v_9(uint start_byte_offset) {
  uint v_10 = (4u + start_byte_offset);
  uint v_11 = (8u + start_byte_offset);
  uint v_12 = (12u + start_byte_offset);
  uint v_13 = (16u + start_byte_offset);
  uint v_14 = (20u + start_byte_offset);
  uint v_15 = (24u + start_byte_offset);
  uint v_16 = (28u + start_byte_offset);
  tint_TransferFunctionParams v_17 = {randomTexture_params[(start_byte_offset / 16u)][((start_byte_offset & 15u) >> 2u)], asfloat(randomTexture_params[(v_10 / 16u)][((v_10 & 15u) >> 2u)]), asfloat(randomTexture_params[(v_11 / 16u)][((v_11 & 15u) >> 2u)]), asfloat(randomTexture_params[(v_12 / 16u)][((v_12 & 15u) >> 2u)]), asfloat(randomTexture_params[(v_13 / 16u)][((v_13 & 15u) >> 2u)]), asfloat(randomTexture_params[(v_14 / 16u)][((v_14 & 15u) >> 2u)]), asfloat(randomTexture_params[(v_15 / 16u)][((v_15 & 15u) >> 2u)]), asfloat(randomTexture_params[(v_16 / 16u)][((v_16 & 15u) >> 2u)])};
  return v_17;
}

float3x4 v_18(uint start_byte_offset) {
  return float3x4(asfloat(randomTexture_params[(start_byte_offset / 16u)]), asfloat(randomTexture_params[((16u + start_byte_offset) / 16u)]), asfloat(randomTexture_params[((32u + start_byte_offset) / 16u)]));
}

tint_ExternalTextureParams v_19(uint start_byte_offset) {
  uint v_20 = randomTexture_params[(start_byte_offset / 16u)][((start_byte_offset & 15u) >> 2u)];
  uint v_21 = (4u + start_byte_offset);
  uint v_22 = randomTexture_params[(v_21 / 16u)][((v_21 & 15u) >> 2u)];
  float3x4 v_23 = v_18((16u + start_byte_offset));
  tint_TransferFunctionParams v_24 = v_9((64u + start_byte_offset));
  tint_TransferFunctionParams v_25 = v_9((96u + start_byte_offset));
  float3x3 v_26 = v_8((128u + start_byte_offset));
  float3x2 v_27 = v((176u + start_byte_offset));
  float3x2 v_28 = v((200u + start_byte_offset));
  uint v_29 = (224u + start_byte_offset);
  uint4 v_30 = randomTexture_params[(v_29 / 16u)];
  float2 v_31 = asfloat((((((v_29 & 15u) >> 2u) == 2u)) ? (v_30.zw) : (v_30.xy)));
  uint v_32 = (232u + start_byte_offset);
  uint4 v_33 = randomTexture_params[(v_32 / 16u)];
  float2 v_34 = asfloat((((((v_32 & 15u) >> 2u) == 2u)) ? (v_33.zw) : (v_33.xy)));
  uint v_35 = (240u + start_byte_offset);
  uint4 v_36 = randomTexture_params[(v_35 / 16u)];
  float2 v_37 = asfloat((((((v_35 & 15u) >> 2u) == 2u)) ? (v_36.zw) : (v_36.xy)));
  uint v_38 = (248u + start_byte_offset);
  uint4 v_39 = randomTexture_params[(v_38 / 16u)];
  float2 v_40 = asfloat((((((v_38 & 15u) >> 2u) == 2u)) ? (v_39.zw) : (v_39.xy)));
  uint v_41 = (256u + start_byte_offset);
  uint4 v_42 = randomTexture_params[(v_41 / 16u)];
  uint2 v_43 = (((((v_41 & 15u) >> 2u) == 2u)) ? (v_42.zw) : (v_42.xy));
  uint v_44 = (264u + start_byte_offset);
  uint4 v_45 = randomTexture_params[(v_44 / 16u)];
  tint_ExternalTextureParams v_46 = {v_20, v_22, v_23, v_24, v_25, v_26, v_27, v_28, v_31, v_34, v_37, v_40, v_43, asfloat((((((v_44 & 15u) >> 2u) == 2u)) ? (v_45.zw) : (v_45.xy)))};
  return v_46;
}

[numthreads(1, 1, 1)]
void main() {
  v_19(0u);
}

//
// main2
//

SamplerState v : register(s1);
Texture2D<float4> depthTexture : register(t2);
[numthreads(1, 1, 1)]
void main2() {
}


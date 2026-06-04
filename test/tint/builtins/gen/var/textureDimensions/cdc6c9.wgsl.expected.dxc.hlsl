//
// fragment_main
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


RWByteAddressBuffer prevent_dce : register(u0);
cbuffer cbuffer_arg_0_params : register(b2, space1) {
  uint4 arg_0_params[17];
};
Texture2D<float4> arg_0_plane0 : register(t0, space1);
Texture2D<float4> arg_0_plane1 : register(t1, space1);
float3x2 v(uint start_byte_offset) {
  uint4 v_1 = arg_0_params[(start_byte_offset / 16u)];
  uint v_2 = (8u + start_byte_offset);
  uint4 v_3 = arg_0_params[(v_2 / 16u)];
  uint v_4 = (16u + start_byte_offset);
  uint4 v_5 = arg_0_params[(v_4 / 16u)];
  return float3x2(asfloat(select((((start_byte_offset & 15u) >> 2u) == 2u), v_1.zw, v_1.xy)), asfloat(select((((v_2 & 15u) >> 2u) == 2u), v_3.zw, v_3.xy)), asfloat(select((((v_4 & 15u) >> 2u) == 2u), v_5.zw, v_5.xy)));
}

float3x3 v_6(uint start_byte_offset) {
  return float3x3(asfloat(arg_0_params[(start_byte_offset / 16u)].xyz), asfloat(arg_0_params[((16u + start_byte_offset) / 16u)].xyz), asfloat(arg_0_params[((32u + start_byte_offset) / 16u)].xyz));
}

tint_TransferFunctionParams v_7(uint start_byte_offset) {
  uint v_8 = (4u + start_byte_offset);
  uint v_9 = (8u + start_byte_offset);
  uint v_10 = (12u + start_byte_offset);
  uint v_11 = (16u + start_byte_offset);
  uint v_12 = (20u + start_byte_offset);
  uint v_13 = (24u + start_byte_offset);
  uint v_14 = (28u + start_byte_offset);
  tint_TransferFunctionParams v_15 = {arg_0_params[(start_byte_offset / 16u)][((start_byte_offset & 15u) >> 2u)], asfloat(arg_0_params[(v_8 / 16u)][((v_8 & 15u) >> 2u)]), asfloat(arg_0_params[(v_9 / 16u)][((v_9 & 15u) >> 2u)]), asfloat(arg_0_params[(v_10 / 16u)][((v_10 & 15u) >> 2u)]), asfloat(arg_0_params[(v_11 / 16u)][((v_11 & 15u) >> 2u)]), asfloat(arg_0_params[(v_12 / 16u)][((v_12 & 15u) >> 2u)]), asfloat(arg_0_params[(v_13 / 16u)][((v_13 & 15u) >> 2u)]), asfloat(arg_0_params[(v_14 / 16u)][((v_14 & 15u) >> 2u)])};
  return v_15;
}

float3x4 v_16(uint start_byte_offset) {
  return float3x4(asfloat(arg_0_params[(start_byte_offset / 16u)]), asfloat(arg_0_params[((16u + start_byte_offset) / 16u)]), asfloat(arg_0_params[((32u + start_byte_offset) / 16u)]));
}

tint_ExternalTextureParams v_17(uint start_byte_offset) {
  uint v_18 = arg_0_params[(start_byte_offset / 16u)][((start_byte_offset & 15u) >> 2u)];
  uint v_19 = (4u + start_byte_offset);
  uint v_20 = arg_0_params[(v_19 / 16u)][((v_19 & 15u) >> 2u)];
  float3x4 v_21 = v_16((16u + start_byte_offset));
  tint_TransferFunctionParams v_22 = v_7((64u + start_byte_offset));
  tint_TransferFunctionParams v_23 = v_7((96u + start_byte_offset));
  float3x3 v_24 = v_6((128u + start_byte_offset));
  float3x2 v_25 = v((176u + start_byte_offset));
  float3x2 v_26 = v((200u + start_byte_offset));
  uint v_27 = (224u + start_byte_offset);
  uint4 v_28 = arg_0_params[(v_27 / 16u)];
  uint v_29 = (232u + start_byte_offset);
  uint4 v_30 = arg_0_params[(v_29 / 16u)];
  uint v_31 = (240u + start_byte_offset);
  uint4 v_32 = arg_0_params[(v_31 / 16u)];
  uint v_33 = (248u + start_byte_offset);
  uint4 v_34 = arg_0_params[(v_33 / 16u)];
  uint v_35 = (256u + start_byte_offset);
  uint4 v_36 = arg_0_params[(v_35 / 16u)];
  uint v_37 = (264u + start_byte_offset);
  uint4 v_38 = arg_0_params[(v_37 / 16u)];
  tint_ExternalTextureParams v_39 = {v_18, v_20, v_21, v_22, v_23, v_24, v_25, v_26, asfloat(select((((v_27 & 15u) >> 2u) == 2u), v_28.zw, v_28.xy)), asfloat(select((((v_29 & 15u) >> 2u) == 2u), v_30.zw, v_30.xy)), asfloat(select((((v_31 & 15u) >> 2u) == 2u), v_32.zw, v_32.xy)), asfloat(select((((v_33 & 15u) >> 2u) == 2u), v_34.zw, v_34.xy)), select((((v_35 & 15u) >> 2u) == 2u), v_36.zw, v_36.xy), asfloat(select((((v_37 & 15u) >> 2u) == 2u), v_38.zw, v_38.xy))};
  return v_39;
}

uint2 textureDimensions_cdc6c9() {
  tint_ExternalTextureParams v_40 = v_17(0u);
  uint2 res = (v_40.apparentSize + (1u).xx);
  return res;
}

void fragment_main() {
  prevent_dce.Store2(0u, textureDimensions_cdc6c9());
}

//
// compute_main
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


RWByteAddressBuffer prevent_dce : register(u0);
cbuffer cbuffer_arg_0_params : register(b2, space1) {
  uint4 arg_0_params[17];
};
Texture2D<float4> arg_0_plane0 : register(t0, space1);
Texture2D<float4> arg_0_plane1 : register(t1, space1);
float3x2 v(uint start_byte_offset) {
  uint4 v_1 = arg_0_params[(start_byte_offset / 16u)];
  uint v_2 = (8u + start_byte_offset);
  uint4 v_3 = arg_0_params[(v_2 / 16u)];
  uint v_4 = (16u + start_byte_offset);
  uint4 v_5 = arg_0_params[(v_4 / 16u)];
  return float3x2(asfloat(select((((start_byte_offset & 15u) >> 2u) == 2u), v_1.zw, v_1.xy)), asfloat(select((((v_2 & 15u) >> 2u) == 2u), v_3.zw, v_3.xy)), asfloat(select((((v_4 & 15u) >> 2u) == 2u), v_5.zw, v_5.xy)));
}

float3x3 v_6(uint start_byte_offset) {
  return float3x3(asfloat(arg_0_params[(start_byte_offset / 16u)].xyz), asfloat(arg_0_params[((16u + start_byte_offset) / 16u)].xyz), asfloat(arg_0_params[((32u + start_byte_offset) / 16u)].xyz));
}

tint_TransferFunctionParams v_7(uint start_byte_offset) {
  uint v_8 = (4u + start_byte_offset);
  uint v_9 = (8u + start_byte_offset);
  uint v_10 = (12u + start_byte_offset);
  uint v_11 = (16u + start_byte_offset);
  uint v_12 = (20u + start_byte_offset);
  uint v_13 = (24u + start_byte_offset);
  uint v_14 = (28u + start_byte_offset);
  tint_TransferFunctionParams v_15 = {arg_0_params[(start_byte_offset / 16u)][((start_byte_offset & 15u) >> 2u)], asfloat(arg_0_params[(v_8 / 16u)][((v_8 & 15u) >> 2u)]), asfloat(arg_0_params[(v_9 / 16u)][((v_9 & 15u) >> 2u)]), asfloat(arg_0_params[(v_10 / 16u)][((v_10 & 15u) >> 2u)]), asfloat(arg_0_params[(v_11 / 16u)][((v_11 & 15u) >> 2u)]), asfloat(arg_0_params[(v_12 / 16u)][((v_12 & 15u) >> 2u)]), asfloat(arg_0_params[(v_13 / 16u)][((v_13 & 15u) >> 2u)]), asfloat(arg_0_params[(v_14 / 16u)][((v_14 & 15u) >> 2u)])};
  return v_15;
}

float3x4 v_16(uint start_byte_offset) {
  return float3x4(asfloat(arg_0_params[(start_byte_offset / 16u)]), asfloat(arg_0_params[((16u + start_byte_offset) / 16u)]), asfloat(arg_0_params[((32u + start_byte_offset) / 16u)]));
}

tint_ExternalTextureParams v_17(uint start_byte_offset) {
  uint v_18 = arg_0_params[(start_byte_offset / 16u)][((start_byte_offset & 15u) >> 2u)];
  uint v_19 = (4u + start_byte_offset);
  uint v_20 = arg_0_params[(v_19 / 16u)][((v_19 & 15u) >> 2u)];
  float3x4 v_21 = v_16((16u + start_byte_offset));
  tint_TransferFunctionParams v_22 = v_7((64u + start_byte_offset));
  tint_TransferFunctionParams v_23 = v_7((96u + start_byte_offset));
  float3x3 v_24 = v_6((128u + start_byte_offset));
  float3x2 v_25 = v((176u + start_byte_offset));
  float3x2 v_26 = v((200u + start_byte_offset));
  uint v_27 = (224u + start_byte_offset);
  uint4 v_28 = arg_0_params[(v_27 / 16u)];
  uint v_29 = (232u + start_byte_offset);
  uint4 v_30 = arg_0_params[(v_29 / 16u)];
  uint v_31 = (240u + start_byte_offset);
  uint4 v_32 = arg_0_params[(v_31 / 16u)];
  uint v_33 = (248u + start_byte_offset);
  uint4 v_34 = arg_0_params[(v_33 / 16u)];
  uint v_35 = (256u + start_byte_offset);
  uint4 v_36 = arg_0_params[(v_35 / 16u)];
  uint v_37 = (264u + start_byte_offset);
  uint4 v_38 = arg_0_params[(v_37 / 16u)];
  tint_ExternalTextureParams v_39 = {v_18, v_20, v_21, v_22, v_23, v_24, v_25, v_26, asfloat(select((((v_27 & 15u) >> 2u) == 2u), v_28.zw, v_28.xy)), asfloat(select((((v_29 & 15u) >> 2u) == 2u), v_30.zw, v_30.xy)), asfloat(select((((v_31 & 15u) >> 2u) == 2u), v_32.zw, v_32.xy)), asfloat(select((((v_33 & 15u) >> 2u) == 2u), v_34.zw, v_34.xy)), select((((v_35 & 15u) >> 2u) == 2u), v_36.zw, v_36.xy), asfloat(select((((v_37 & 15u) >> 2u) == 2u), v_38.zw, v_38.xy))};
  return v_39;
}

uint2 textureDimensions_cdc6c9() {
  tint_ExternalTextureParams v_40 = v_17(0u);
  uint2 res = (v_40.apparentSize + (1u).xx);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store2(0u, textureDimensions_cdc6c9());
}

//
// vertex_main
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

struct VertexOutput {
  float4 pos;
  uint2 prevent_dce;
};

struct vertex_main_outputs {
  nointerpolation uint2 VertexOutput_prevent_dce : TEXCOORD0;
  float4 VertexOutput_pos : SV_Position;
};


cbuffer cbuffer_arg_0_params : register(b2, space1) {
  uint4 arg_0_params[17];
};
Texture2D<float4> arg_0_plane0 : register(t0, space1);
Texture2D<float4> arg_0_plane1 : register(t1, space1);
float3x2 v(uint start_byte_offset) {
  uint4 v_1 = arg_0_params[(start_byte_offset / 16u)];
  uint v_2 = (8u + start_byte_offset);
  uint4 v_3 = arg_0_params[(v_2 / 16u)];
  uint v_4 = (16u + start_byte_offset);
  uint4 v_5 = arg_0_params[(v_4 / 16u)];
  return float3x2(asfloat(select((((start_byte_offset & 15u) >> 2u) == 2u), v_1.zw, v_1.xy)), asfloat(select((((v_2 & 15u) >> 2u) == 2u), v_3.zw, v_3.xy)), asfloat(select((((v_4 & 15u) >> 2u) == 2u), v_5.zw, v_5.xy)));
}

float3x3 v_6(uint start_byte_offset) {
  return float3x3(asfloat(arg_0_params[(start_byte_offset / 16u)].xyz), asfloat(arg_0_params[((16u + start_byte_offset) / 16u)].xyz), asfloat(arg_0_params[((32u + start_byte_offset) / 16u)].xyz));
}

tint_TransferFunctionParams v_7(uint start_byte_offset) {
  uint v_8 = (4u + start_byte_offset);
  uint v_9 = (8u + start_byte_offset);
  uint v_10 = (12u + start_byte_offset);
  uint v_11 = (16u + start_byte_offset);
  uint v_12 = (20u + start_byte_offset);
  uint v_13 = (24u + start_byte_offset);
  uint v_14 = (28u + start_byte_offset);
  tint_TransferFunctionParams v_15 = {arg_0_params[(start_byte_offset / 16u)][((start_byte_offset & 15u) >> 2u)], asfloat(arg_0_params[(v_8 / 16u)][((v_8 & 15u) >> 2u)]), asfloat(arg_0_params[(v_9 / 16u)][((v_9 & 15u) >> 2u)]), asfloat(arg_0_params[(v_10 / 16u)][((v_10 & 15u) >> 2u)]), asfloat(arg_0_params[(v_11 / 16u)][((v_11 & 15u) >> 2u)]), asfloat(arg_0_params[(v_12 / 16u)][((v_12 & 15u) >> 2u)]), asfloat(arg_0_params[(v_13 / 16u)][((v_13 & 15u) >> 2u)]), asfloat(arg_0_params[(v_14 / 16u)][((v_14 & 15u) >> 2u)])};
  return v_15;
}

float3x4 v_16(uint start_byte_offset) {
  return float3x4(asfloat(arg_0_params[(start_byte_offset / 16u)]), asfloat(arg_0_params[((16u + start_byte_offset) / 16u)]), asfloat(arg_0_params[((32u + start_byte_offset) / 16u)]));
}

tint_ExternalTextureParams v_17(uint start_byte_offset) {
  uint v_18 = arg_0_params[(start_byte_offset / 16u)][((start_byte_offset & 15u) >> 2u)];
  uint v_19 = (4u + start_byte_offset);
  uint v_20 = arg_0_params[(v_19 / 16u)][((v_19 & 15u) >> 2u)];
  float3x4 v_21 = v_16((16u + start_byte_offset));
  tint_TransferFunctionParams v_22 = v_7((64u + start_byte_offset));
  tint_TransferFunctionParams v_23 = v_7((96u + start_byte_offset));
  float3x3 v_24 = v_6((128u + start_byte_offset));
  float3x2 v_25 = v((176u + start_byte_offset));
  float3x2 v_26 = v((200u + start_byte_offset));
  uint v_27 = (224u + start_byte_offset);
  uint4 v_28 = arg_0_params[(v_27 / 16u)];
  uint v_29 = (232u + start_byte_offset);
  uint4 v_30 = arg_0_params[(v_29 / 16u)];
  uint v_31 = (240u + start_byte_offset);
  uint4 v_32 = arg_0_params[(v_31 / 16u)];
  uint v_33 = (248u + start_byte_offset);
  uint4 v_34 = arg_0_params[(v_33 / 16u)];
  uint v_35 = (256u + start_byte_offset);
  uint4 v_36 = arg_0_params[(v_35 / 16u)];
  uint v_37 = (264u + start_byte_offset);
  uint4 v_38 = arg_0_params[(v_37 / 16u)];
  tint_ExternalTextureParams v_39 = {v_18, v_20, v_21, v_22, v_23, v_24, v_25, v_26, asfloat(select((((v_27 & 15u) >> 2u) == 2u), v_28.zw, v_28.xy)), asfloat(select((((v_29 & 15u) >> 2u) == 2u), v_30.zw, v_30.xy)), asfloat(select((((v_31 & 15u) >> 2u) == 2u), v_32.zw, v_32.xy)), asfloat(select((((v_33 & 15u) >> 2u) == 2u), v_34.zw, v_34.xy)), select((((v_35 & 15u) >> 2u) == 2u), v_36.zw, v_36.xy), asfloat(select((((v_37 & 15u) >> 2u) == 2u), v_38.zw, v_38.xy))};
  return v_39;
}

uint2 textureDimensions_cdc6c9() {
  tint_ExternalTextureParams v_40 = v_17(0u);
  uint2 res = (v_40.apparentSize + (1u).xx);
  return res;
}

VertexOutput vertex_main_inner() {
  VertexOutput v_41 = (VertexOutput)0;
  v_41.pos = (0.0f).xxxx;
  v_41.prevent_dce = textureDimensions_cdc6c9();
  VertexOutput v_42 = v_41;
  return v_42;
}

vertex_main_outputs vertex_main() {
  VertexOutput v_43 = vertex_main_inner();
  vertex_main_outputs v_44 = {v_43.prevent_dce, v_43.pos};
  return v_44;
}


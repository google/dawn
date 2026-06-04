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
  float2 v_2 = asfloat((((((start_byte_offset & 15u) >> 2u) == 2u)) ? (v_1.zw) : (v_1.xy)));
  uint v_3 = (8u + start_byte_offset);
  uint4 v_4 = arg_0_params[(v_3 / 16u)];
  float2 v_5 = asfloat((((((v_3 & 15u) >> 2u) == 2u)) ? (v_4.zw) : (v_4.xy)));
  uint v_6 = (16u + start_byte_offset);
  uint4 v_7 = arg_0_params[(v_6 / 16u)];
  return float3x2(v_2, v_5, asfloat((((((v_6 & 15u) >> 2u) == 2u)) ? (v_7.zw) : (v_7.xy))));
}

float3x3 v_8(uint start_byte_offset) {
  return float3x3(asfloat(arg_0_params[(start_byte_offset / 16u)].xyz), asfloat(arg_0_params[((16u + start_byte_offset) / 16u)].xyz), asfloat(arg_0_params[((32u + start_byte_offset) / 16u)].xyz));
}

tint_TransferFunctionParams v_9(uint start_byte_offset) {
  uint v_10 = (4u + start_byte_offset);
  uint v_11 = (8u + start_byte_offset);
  uint v_12 = (12u + start_byte_offset);
  uint v_13 = (16u + start_byte_offset);
  uint v_14 = (20u + start_byte_offset);
  uint v_15 = (24u + start_byte_offset);
  uint v_16 = (28u + start_byte_offset);
  tint_TransferFunctionParams v_17 = {arg_0_params[(start_byte_offset / 16u)][((start_byte_offset & 15u) >> 2u)], asfloat(arg_0_params[(v_10 / 16u)][((v_10 & 15u) >> 2u)]), asfloat(arg_0_params[(v_11 / 16u)][((v_11 & 15u) >> 2u)]), asfloat(arg_0_params[(v_12 / 16u)][((v_12 & 15u) >> 2u)]), asfloat(arg_0_params[(v_13 / 16u)][((v_13 & 15u) >> 2u)]), asfloat(arg_0_params[(v_14 / 16u)][((v_14 & 15u) >> 2u)]), asfloat(arg_0_params[(v_15 / 16u)][((v_15 & 15u) >> 2u)]), asfloat(arg_0_params[(v_16 / 16u)][((v_16 & 15u) >> 2u)])};
  return v_17;
}

float3x4 v_18(uint start_byte_offset) {
  return float3x4(asfloat(arg_0_params[(start_byte_offset / 16u)]), asfloat(arg_0_params[((16u + start_byte_offset) / 16u)]), asfloat(arg_0_params[((32u + start_byte_offset) / 16u)]));
}

tint_ExternalTextureParams v_19(uint start_byte_offset) {
  uint v_20 = arg_0_params[(start_byte_offset / 16u)][((start_byte_offset & 15u) >> 2u)];
  uint v_21 = (4u + start_byte_offset);
  uint v_22 = arg_0_params[(v_21 / 16u)][((v_21 & 15u) >> 2u)];
  float3x4 v_23 = v_18((16u + start_byte_offset));
  tint_TransferFunctionParams v_24 = v_9((64u + start_byte_offset));
  tint_TransferFunctionParams v_25 = v_9((96u + start_byte_offset));
  float3x3 v_26 = v_8((128u + start_byte_offset));
  float3x2 v_27 = v((176u + start_byte_offset));
  float3x2 v_28 = v((200u + start_byte_offset));
  uint v_29 = (224u + start_byte_offset);
  uint4 v_30 = arg_0_params[(v_29 / 16u)];
  float2 v_31 = asfloat((((((v_29 & 15u) >> 2u) == 2u)) ? (v_30.zw) : (v_30.xy)));
  uint v_32 = (232u + start_byte_offset);
  uint4 v_33 = arg_0_params[(v_32 / 16u)];
  float2 v_34 = asfloat((((((v_32 & 15u) >> 2u) == 2u)) ? (v_33.zw) : (v_33.xy)));
  uint v_35 = (240u + start_byte_offset);
  uint4 v_36 = arg_0_params[(v_35 / 16u)];
  float2 v_37 = asfloat((((((v_35 & 15u) >> 2u) == 2u)) ? (v_36.zw) : (v_36.xy)));
  uint v_38 = (248u + start_byte_offset);
  uint4 v_39 = arg_0_params[(v_38 / 16u)];
  float2 v_40 = asfloat((((((v_38 & 15u) >> 2u) == 2u)) ? (v_39.zw) : (v_39.xy)));
  uint v_41 = (256u + start_byte_offset);
  uint4 v_42 = arg_0_params[(v_41 / 16u)];
  uint2 v_43 = (((((v_41 & 15u) >> 2u) == 2u)) ? (v_42.zw) : (v_42.xy));
  uint v_44 = (264u + start_byte_offset);
  uint4 v_45 = arg_0_params[(v_44 / 16u)];
  tint_ExternalTextureParams v_46 = {v_20, v_22, v_23, v_24, v_25, v_26, v_27, v_28, v_31, v_34, v_37, v_40, v_43, asfloat((((((v_44 & 15u) >> 2u) == 2u)) ? (v_45.zw) : (v_45.xy)))};
  return v_46;
}

uint2 textureDimensions_cdc6c9() {
  tint_ExternalTextureParams v_47 = v_19(0u);
  uint2 res = (v_47.apparentSize + (1u).xx);
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
  float2 v_2 = asfloat((((((start_byte_offset & 15u) >> 2u) == 2u)) ? (v_1.zw) : (v_1.xy)));
  uint v_3 = (8u + start_byte_offset);
  uint4 v_4 = arg_0_params[(v_3 / 16u)];
  float2 v_5 = asfloat((((((v_3 & 15u) >> 2u) == 2u)) ? (v_4.zw) : (v_4.xy)));
  uint v_6 = (16u + start_byte_offset);
  uint4 v_7 = arg_0_params[(v_6 / 16u)];
  return float3x2(v_2, v_5, asfloat((((((v_6 & 15u) >> 2u) == 2u)) ? (v_7.zw) : (v_7.xy))));
}

float3x3 v_8(uint start_byte_offset) {
  return float3x3(asfloat(arg_0_params[(start_byte_offset / 16u)].xyz), asfloat(arg_0_params[((16u + start_byte_offset) / 16u)].xyz), asfloat(arg_0_params[((32u + start_byte_offset) / 16u)].xyz));
}

tint_TransferFunctionParams v_9(uint start_byte_offset) {
  uint v_10 = (4u + start_byte_offset);
  uint v_11 = (8u + start_byte_offset);
  uint v_12 = (12u + start_byte_offset);
  uint v_13 = (16u + start_byte_offset);
  uint v_14 = (20u + start_byte_offset);
  uint v_15 = (24u + start_byte_offset);
  uint v_16 = (28u + start_byte_offset);
  tint_TransferFunctionParams v_17 = {arg_0_params[(start_byte_offset / 16u)][((start_byte_offset & 15u) >> 2u)], asfloat(arg_0_params[(v_10 / 16u)][((v_10 & 15u) >> 2u)]), asfloat(arg_0_params[(v_11 / 16u)][((v_11 & 15u) >> 2u)]), asfloat(arg_0_params[(v_12 / 16u)][((v_12 & 15u) >> 2u)]), asfloat(arg_0_params[(v_13 / 16u)][((v_13 & 15u) >> 2u)]), asfloat(arg_0_params[(v_14 / 16u)][((v_14 & 15u) >> 2u)]), asfloat(arg_0_params[(v_15 / 16u)][((v_15 & 15u) >> 2u)]), asfloat(arg_0_params[(v_16 / 16u)][((v_16 & 15u) >> 2u)])};
  return v_17;
}

float3x4 v_18(uint start_byte_offset) {
  return float3x4(asfloat(arg_0_params[(start_byte_offset / 16u)]), asfloat(arg_0_params[((16u + start_byte_offset) / 16u)]), asfloat(arg_0_params[((32u + start_byte_offset) / 16u)]));
}

tint_ExternalTextureParams v_19(uint start_byte_offset) {
  uint v_20 = arg_0_params[(start_byte_offset / 16u)][((start_byte_offset & 15u) >> 2u)];
  uint v_21 = (4u + start_byte_offset);
  uint v_22 = arg_0_params[(v_21 / 16u)][((v_21 & 15u) >> 2u)];
  float3x4 v_23 = v_18((16u + start_byte_offset));
  tint_TransferFunctionParams v_24 = v_9((64u + start_byte_offset));
  tint_TransferFunctionParams v_25 = v_9((96u + start_byte_offset));
  float3x3 v_26 = v_8((128u + start_byte_offset));
  float3x2 v_27 = v((176u + start_byte_offset));
  float3x2 v_28 = v((200u + start_byte_offset));
  uint v_29 = (224u + start_byte_offset);
  uint4 v_30 = arg_0_params[(v_29 / 16u)];
  float2 v_31 = asfloat((((((v_29 & 15u) >> 2u) == 2u)) ? (v_30.zw) : (v_30.xy)));
  uint v_32 = (232u + start_byte_offset);
  uint4 v_33 = arg_0_params[(v_32 / 16u)];
  float2 v_34 = asfloat((((((v_32 & 15u) >> 2u) == 2u)) ? (v_33.zw) : (v_33.xy)));
  uint v_35 = (240u + start_byte_offset);
  uint4 v_36 = arg_0_params[(v_35 / 16u)];
  float2 v_37 = asfloat((((((v_35 & 15u) >> 2u) == 2u)) ? (v_36.zw) : (v_36.xy)));
  uint v_38 = (248u + start_byte_offset);
  uint4 v_39 = arg_0_params[(v_38 / 16u)];
  float2 v_40 = asfloat((((((v_38 & 15u) >> 2u) == 2u)) ? (v_39.zw) : (v_39.xy)));
  uint v_41 = (256u + start_byte_offset);
  uint4 v_42 = arg_0_params[(v_41 / 16u)];
  uint2 v_43 = (((((v_41 & 15u) >> 2u) == 2u)) ? (v_42.zw) : (v_42.xy));
  uint v_44 = (264u + start_byte_offset);
  uint4 v_45 = arg_0_params[(v_44 / 16u)];
  tint_ExternalTextureParams v_46 = {v_20, v_22, v_23, v_24, v_25, v_26, v_27, v_28, v_31, v_34, v_37, v_40, v_43, asfloat((((((v_44 & 15u) >> 2u) == 2u)) ? (v_45.zw) : (v_45.xy)))};
  return v_46;
}

uint2 textureDimensions_cdc6c9() {
  tint_ExternalTextureParams v_47 = v_19(0u);
  uint2 res = (v_47.apparentSize + (1u).xx);
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
  float2 v_2 = asfloat((((((start_byte_offset & 15u) >> 2u) == 2u)) ? (v_1.zw) : (v_1.xy)));
  uint v_3 = (8u + start_byte_offset);
  uint4 v_4 = arg_0_params[(v_3 / 16u)];
  float2 v_5 = asfloat((((((v_3 & 15u) >> 2u) == 2u)) ? (v_4.zw) : (v_4.xy)));
  uint v_6 = (16u + start_byte_offset);
  uint4 v_7 = arg_0_params[(v_6 / 16u)];
  return float3x2(v_2, v_5, asfloat((((((v_6 & 15u) >> 2u) == 2u)) ? (v_7.zw) : (v_7.xy))));
}

float3x3 v_8(uint start_byte_offset) {
  return float3x3(asfloat(arg_0_params[(start_byte_offset / 16u)].xyz), asfloat(arg_0_params[((16u + start_byte_offset) / 16u)].xyz), asfloat(arg_0_params[((32u + start_byte_offset) / 16u)].xyz));
}

tint_TransferFunctionParams v_9(uint start_byte_offset) {
  uint v_10 = (4u + start_byte_offset);
  uint v_11 = (8u + start_byte_offset);
  uint v_12 = (12u + start_byte_offset);
  uint v_13 = (16u + start_byte_offset);
  uint v_14 = (20u + start_byte_offset);
  uint v_15 = (24u + start_byte_offset);
  uint v_16 = (28u + start_byte_offset);
  tint_TransferFunctionParams v_17 = {arg_0_params[(start_byte_offset / 16u)][((start_byte_offset & 15u) >> 2u)], asfloat(arg_0_params[(v_10 / 16u)][((v_10 & 15u) >> 2u)]), asfloat(arg_0_params[(v_11 / 16u)][((v_11 & 15u) >> 2u)]), asfloat(arg_0_params[(v_12 / 16u)][((v_12 & 15u) >> 2u)]), asfloat(arg_0_params[(v_13 / 16u)][((v_13 & 15u) >> 2u)]), asfloat(arg_0_params[(v_14 / 16u)][((v_14 & 15u) >> 2u)]), asfloat(arg_0_params[(v_15 / 16u)][((v_15 & 15u) >> 2u)]), asfloat(arg_0_params[(v_16 / 16u)][((v_16 & 15u) >> 2u)])};
  return v_17;
}

float3x4 v_18(uint start_byte_offset) {
  return float3x4(asfloat(arg_0_params[(start_byte_offset / 16u)]), asfloat(arg_0_params[((16u + start_byte_offset) / 16u)]), asfloat(arg_0_params[((32u + start_byte_offset) / 16u)]));
}

tint_ExternalTextureParams v_19(uint start_byte_offset) {
  uint v_20 = arg_0_params[(start_byte_offset / 16u)][((start_byte_offset & 15u) >> 2u)];
  uint v_21 = (4u + start_byte_offset);
  uint v_22 = arg_0_params[(v_21 / 16u)][((v_21 & 15u) >> 2u)];
  float3x4 v_23 = v_18((16u + start_byte_offset));
  tint_TransferFunctionParams v_24 = v_9((64u + start_byte_offset));
  tint_TransferFunctionParams v_25 = v_9((96u + start_byte_offset));
  float3x3 v_26 = v_8((128u + start_byte_offset));
  float3x2 v_27 = v((176u + start_byte_offset));
  float3x2 v_28 = v((200u + start_byte_offset));
  uint v_29 = (224u + start_byte_offset);
  uint4 v_30 = arg_0_params[(v_29 / 16u)];
  float2 v_31 = asfloat((((((v_29 & 15u) >> 2u) == 2u)) ? (v_30.zw) : (v_30.xy)));
  uint v_32 = (232u + start_byte_offset);
  uint4 v_33 = arg_0_params[(v_32 / 16u)];
  float2 v_34 = asfloat((((((v_32 & 15u) >> 2u) == 2u)) ? (v_33.zw) : (v_33.xy)));
  uint v_35 = (240u + start_byte_offset);
  uint4 v_36 = arg_0_params[(v_35 / 16u)];
  float2 v_37 = asfloat((((((v_35 & 15u) >> 2u) == 2u)) ? (v_36.zw) : (v_36.xy)));
  uint v_38 = (248u + start_byte_offset);
  uint4 v_39 = arg_0_params[(v_38 / 16u)];
  float2 v_40 = asfloat((((((v_38 & 15u) >> 2u) == 2u)) ? (v_39.zw) : (v_39.xy)));
  uint v_41 = (256u + start_byte_offset);
  uint4 v_42 = arg_0_params[(v_41 / 16u)];
  uint2 v_43 = (((((v_41 & 15u) >> 2u) == 2u)) ? (v_42.zw) : (v_42.xy));
  uint v_44 = (264u + start_byte_offset);
  uint4 v_45 = arg_0_params[(v_44 / 16u)];
  tint_ExternalTextureParams v_46 = {v_20, v_22, v_23, v_24, v_25, v_26, v_27, v_28, v_31, v_34, v_37, v_40, v_43, asfloat((((((v_44 & 15u) >> 2u) == 2u)) ? (v_45.zw) : (v_45.xy)))};
  return v_46;
}

uint2 textureDimensions_cdc6c9() {
  tint_ExternalTextureParams v_47 = v_19(0u);
  uint2 res = (v_47.apparentSize + (1u).xx);
  return res;
}

VertexOutput vertex_main_inner() {
  VertexOutput v_48 = (VertexOutput)0;
  v_48.pos = (0.0f).xxxx;
  v_48.prevent_dce = textureDimensions_cdc6c9();
  VertexOutput v_49 = v_48;
  return v_49;
}

vertex_main_outputs vertex_main() {
  VertexOutput v_50 = vertex_main_inner();
  vertex_main_outputs v_51 = {v_50.prevent_dce, v_50.pos};
  return v_51;
}


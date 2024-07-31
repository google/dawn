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
  uint2 visibleSize;
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


RWByteAddressBuffer prevent_dce : register(u0);
Texture2D<float4> arg_0_plane0 : register(t0, space1);
Texture2D<float4> arg_0_plane1 : register(t1, space1);
cbuffer cbuffer_arg_0_params : register(b2, space1) {
  uint4 arg_0_params[17];
};
float3x2 v(uint start_byte_offset) {
  uint4 v_1 = arg_0_params[(start_byte_offset / 16u)];
  float2 v_2 = asfloat((((((start_byte_offset % 16u) / 4u) == 2u)) ? (v_1.zw) : (v_1.xy)));
  uint4 v_3 = arg_0_params[((8u + start_byte_offset) / 16u)];
  float2 v_4 = asfloat(((((((8u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_3.zw) : (v_3.xy)));
  uint4 v_5 = arg_0_params[((16u + start_byte_offset) / 16u)];
  return float3x2(v_2, v_4, asfloat(((((((16u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_5.zw) : (v_5.xy))));
}

float3x3 v_6(uint start_byte_offset) {
  float3 v_7 = asfloat(arg_0_params[(start_byte_offset / 16u)].xyz);
  float3 v_8 = asfloat(arg_0_params[((16u + start_byte_offset) / 16u)].xyz);
  return float3x3(v_7, v_8, asfloat(arg_0_params[((32u + start_byte_offset) / 16u)].xyz));
}

tint_GammaTransferParams v_9(uint start_byte_offset) {
  float v_10 = asfloat(arg_0_params[(start_byte_offset / 16u)][((start_byte_offset % 16u) / 4u)]);
  float v_11 = asfloat(arg_0_params[((4u + start_byte_offset) / 16u)][(((4u + start_byte_offset) % 16u) / 4u)]);
  float v_12 = asfloat(arg_0_params[((8u + start_byte_offset) / 16u)][(((8u + start_byte_offset) % 16u) / 4u)]);
  float v_13 = asfloat(arg_0_params[((12u + start_byte_offset) / 16u)][(((12u + start_byte_offset) % 16u) / 4u)]);
  float v_14 = asfloat(arg_0_params[((16u + start_byte_offset) / 16u)][(((16u + start_byte_offset) % 16u) / 4u)]);
  float v_15 = asfloat(arg_0_params[((20u + start_byte_offset) / 16u)][(((20u + start_byte_offset) % 16u) / 4u)]);
  float v_16 = asfloat(arg_0_params[((24u + start_byte_offset) / 16u)][(((24u + start_byte_offset) % 16u) / 4u)]);
  tint_GammaTransferParams v_17 = {v_10, v_11, v_12, v_13, v_14, v_15, v_16, arg_0_params[((28u + start_byte_offset) / 16u)][(((28u + start_byte_offset) % 16u) / 4u)]};
  return v_17;
}

float3x4 v_18(uint start_byte_offset) {
  float4 v_19 = asfloat(arg_0_params[(start_byte_offset / 16u)]);
  float4 v_20 = asfloat(arg_0_params[((16u + start_byte_offset) / 16u)]);
  return float3x4(v_19, v_20, asfloat(arg_0_params[((32u + start_byte_offset) / 16u)]));
}

tint_ExternalTextureParams v_21(uint start_byte_offset) {
  uint v_22 = arg_0_params[(start_byte_offset / 16u)][((start_byte_offset % 16u) / 4u)];
  uint v_23 = arg_0_params[((4u + start_byte_offset) / 16u)][(((4u + start_byte_offset) % 16u) / 4u)];
  float3x4 v_24 = v_18((16u + start_byte_offset));
  tint_GammaTransferParams v_25 = v_9((64u + start_byte_offset));
  tint_GammaTransferParams v_26 = v_9((96u + start_byte_offset));
  float3x3 v_27 = v_6((128u + start_byte_offset));
  float3x2 v_28 = v((176u + start_byte_offset));
  float3x2 v_29 = v((200u + start_byte_offset));
  uint4 v_30 = arg_0_params[((224u + start_byte_offset) / 16u)];
  float2 v_31 = asfloat(((((((224u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_30.zw) : (v_30.xy)));
  uint4 v_32 = arg_0_params[((232u + start_byte_offset) / 16u)];
  float2 v_33 = asfloat(((((((232u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_32.zw) : (v_32.xy)));
  uint4 v_34 = arg_0_params[((240u + start_byte_offset) / 16u)];
  float2 v_35 = asfloat(((((((240u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_34.zw) : (v_34.xy)));
  uint4 v_36 = arg_0_params[((248u + start_byte_offset) / 16u)];
  float2 v_37 = asfloat(((((((248u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_36.zw) : (v_36.xy)));
  uint4 v_38 = arg_0_params[((256u + start_byte_offset) / 16u)];
  uint2 v_39 = ((((((256u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_38.zw) : (v_38.xy));
  uint4 v_40 = arg_0_params[((264u + start_byte_offset) / 16u)];
  tint_GammaTransferParams v_41 = v_25;
  tint_GammaTransferParams v_42 = v_26;
  tint_ExternalTextureParams v_43 = {v_22, v_23, v_24, v_41, v_42, v_27, v_28, v_29, v_31, v_33, v_35, v_37, v_39, asfloat(((((((264u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_40.zw) : (v_40.xy)))};
  return v_43;
}

uint2 textureDimensions_cdc6c9() {
  tint_ExternalTextureParams v_44 = v_21(0u);
  uint2 res = (v_44.visibleSize + (1u).xx);
  return res;
}

void fragment_main() {
  prevent_dce.Store2(0u, textureDimensions_cdc6c9());
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store2(0u, textureDimensions_cdc6c9());
}

VertexOutput vertex_main_inner() {
  VertexOutput tint_symbol = (VertexOutput)0;
  tint_symbol.pos = (0.0f).xxxx;
  tint_symbol.prevent_dce = textureDimensions_cdc6c9();
  VertexOutput v_45 = tint_symbol;
  return v_45;
}

vertex_main_outputs vertex_main() {
  VertexOutput v_46 = vertex_main_inner();
  VertexOutput v_47 = v_46;
  VertexOutput v_48 = v_46;
  vertex_main_outputs v_49 = {v_48.prevent_dce, v_47.pos};
  return v_49;
}


#version 310 es
precision highp float;
precision highp int;


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

struct tint_ExternalTextureParams_std140 {
  uint numPlanes;
  uint doYuvToRgbConversionOnly;
  uint tint_pad_0;
  uint tint_pad_1;
  mat3x4 yuvToRgbConversionMatrix;
  tint_GammaTransferParams gammaDecodeParams;
  tint_GammaTransferParams gammaEncodeParams;
  vec3 gamutConversionMatrix_col0;
  uint tint_pad_2;
  vec3 gamutConversionMatrix_col1;
  uint tint_pad_3;
  vec3 gamutConversionMatrix_col2;
  uint tint_pad_4;
  vec2 sampleTransform_col0;
  vec2 sampleTransform_col1;
  vec2 sampleTransform_col2;
  vec2 loadTransform_col0;
  vec2 loadTransform_col1;
  vec2 loadTransform_col2;
  vec2 samplePlane0RectMin;
  vec2 samplePlane0RectMax;
  vec2 samplePlane1RectMin;
  vec2 samplePlane1RectMax;
  uvec2 visibleSize;
  vec2 plane1CoordFactor;
};

struct tint_ExternalTextureParams {
  uint numPlanes;
  uint doYuvToRgbConversionOnly;
  mat3x4 yuvToRgbConversionMatrix;
  tint_GammaTransferParams gammaDecodeParams;
  tint_GammaTransferParams gammaEncodeParams;
  mat3 gamutConversionMatrix;
  mat3x2 sampleTransform;
  mat3x2 loadTransform;
  vec2 samplePlane0RectMin;
  vec2 samplePlane0RectMax;
  vec2 samplePlane1RectMin;
  vec2 samplePlane1RectMax;
  uvec2 visibleSize;
  vec2 plane1CoordFactor;
};

layout(binding = 0, std430)
buffer prevent_dce_block_1_ssbo {
  vec4 inner;
} v_1;
layout(binding = 3, std140)
uniform arg_0_params_block_std140_1_ubo {
  tint_ExternalTextureParams_std140 inner;
} v_2;
uniform highp sampler2D arg_0_plane0_arg_1;
uniform highp sampler2D arg_0_plane1_arg_1;
vec3 tint_GammaCorrection(vec3 v, tint_GammaTransferParams params) {
  vec3 v_3 = vec3(params.G);
  vec3 v_4 = vec3(params.D);
  vec3 v_5 = abs(v);
  vec3 v_6 = sign(v);
  bvec3 v_7 = lessThan(v_5, v_4);
  return mix((v_6 * (pow(((params.A * v_5) + params.B), v_3) + params.E)), (v_6 * ((params.C * v_5) + params.F)), v_7);
}
vec4 tint_TextureSampleExternal(tint_ExternalTextureParams params, vec2 coords) {
  vec2 v_8 = (params.sampleTransform * vec3(coords, 1.0f));
  vec2 v_9 = clamp(v_8, params.samplePlane0RectMin, params.samplePlane0RectMax);
  vec3 v_10 = vec3(0.0f);
  float v_11 = 0.0f;
  if ((params.numPlanes == 1u)) {
    vec4 v_12 = textureLod(arg_0_plane0_arg_1, v_9, float(0.0f));
    v_10 = v_12.xyz;
    v_11 = v_12[3u];
  } else {
    float v_13 = textureLod(arg_0_plane0_arg_1, v_9, float(0.0f))[0u];
    vec2 v_14 = clamp(v_8, params.samplePlane1RectMin, params.samplePlane1RectMax);
    v_10 = (vec4(v_13, textureLod(arg_0_plane1_arg_1, v_14, float(0.0f)).xy, 1.0f) * params.yuvToRgbConversionMatrix);
    v_11 = 1.0f;
  }
  vec3 v_15 = v_10;
  vec3 v_16 = vec3(0.0f);
  if ((params.doYuvToRgbConversionOnly == 0u)) {
    v_16 = tint_GammaCorrection((params.gamutConversionMatrix * tint_GammaCorrection(v_15, params.gammaDecodeParams)), params.gammaEncodeParams);
  } else {
    v_16 = v_15;
  }
  return vec4(v_16, v_11);
}
tint_ExternalTextureParams tint_convert_tint_ExternalTextureParams(tint_ExternalTextureParams_std140 tint_input) {
  mat3 v_17 = mat3(tint_input.gamutConversionMatrix_col0, tint_input.gamutConversionMatrix_col1, tint_input.gamutConversionMatrix_col2);
  mat3x2 v_18 = mat3x2(tint_input.sampleTransform_col0, tint_input.sampleTransform_col1, tint_input.sampleTransform_col2);
  return tint_ExternalTextureParams(tint_input.numPlanes, tint_input.doYuvToRgbConversionOnly, tint_input.yuvToRgbConversionMatrix, tint_input.gammaDecodeParams, tint_input.gammaEncodeParams, v_17, v_18, mat3x2(tint_input.loadTransform_col0, tint_input.loadTransform_col1, tint_input.loadTransform_col2), tint_input.samplePlane0RectMin, tint_input.samplePlane0RectMax, tint_input.samplePlane1RectMin, tint_input.samplePlane1RectMax, tint_input.visibleSize, tint_input.plane1CoordFactor);
}
vec4 textureSampleBaseClampToEdge_7c04e6() {
  vec2 arg_2 = vec2(1.0f);
  tint_ExternalTextureParams v_19 = tint_convert_tint_ExternalTextureParams(v_2.inner);
  vec4 res = tint_TextureSampleExternal(v_19, arg_2);
  return res;
}
void main() {
  v_1.inner = textureSampleBaseClampToEdge_7c04e6();
}
#version 310 es


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

struct tint_ExternalTextureParams_std140 {
  uint numPlanes;
  uint doYuvToRgbConversionOnly;
  uint tint_pad_0;
  uint tint_pad_1;
  mat3x4 yuvToRgbConversionMatrix;
  tint_GammaTransferParams gammaDecodeParams;
  tint_GammaTransferParams gammaEncodeParams;
  vec3 gamutConversionMatrix_col0;
  uint tint_pad_2;
  vec3 gamutConversionMatrix_col1;
  uint tint_pad_3;
  vec3 gamutConversionMatrix_col2;
  uint tint_pad_4;
  vec2 sampleTransform_col0;
  vec2 sampleTransform_col1;
  vec2 sampleTransform_col2;
  vec2 loadTransform_col0;
  vec2 loadTransform_col1;
  vec2 loadTransform_col2;
  vec2 samplePlane0RectMin;
  vec2 samplePlane0RectMax;
  vec2 samplePlane1RectMin;
  vec2 samplePlane1RectMax;
  uvec2 visibleSize;
  vec2 plane1CoordFactor;
};

struct tint_ExternalTextureParams {
  uint numPlanes;
  uint doYuvToRgbConversionOnly;
  mat3x4 yuvToRgbConversionMatrix;
  tint_GammaTransferParams gammaDecodeParams;
  tint_GammaTransferParams gammaEncodeParams;
  mat3 gamutConversionMatrix;
  mat3x2 sampleTransform;
  mat3x2 loadTransform;
  vec2 samplePlane0RectMin;
  vec2 samplePlane0RectMax;
  vec2 samplePlane1RectMin;
  vec2 samplePlane1RectMax;
  uvec2 visibleSize;
  vec2 plane1CoordFactor;
};

layout(binding = 0, std430)
buffer prevent_dce_block_1_ssbo {
  vec4 inner;
} v_1;
layout(binding = 3, std140)
uniform arg_0_params_block_std140_1_ubo {
  tint_ExternalTextureParams_std140 inner;
} v_2;
uniform highp sampler2D arg_0_plane0_arg_1;
uniform highp sampler2D arg_0_plane1_arg_1;
vec3 tint_GammaCorrection(vec3 v, tint_GammaTransferParams params) {
  vec3 v_3 = vec3(params.G);
  vec3 v_4 = vec3(params.D);
  vec3 v_5 = abs(v);
  vec3 v_6 = sign(v);
  bvec3 v_7 = lessThan(v_5, v_4);
  return mix((v_6 * (pow(((params.A * v_5) + params.B), v_3) + params.E)), (v_6 * ((params.C * v_5) + params.F)), v_7);
}
vec4 tint_TextureSampleExternal(tint_ExternalTextureParams params, vec2 coords) {
  vec2 v_8 = (params.sampleTransform * vec3(coords, 1.0f));
  vec2 v_9 = clamp(v_8, params.samplePlane0RectMin, params.samplePlane0RectMax);
  vec3 v_10 = vec3(0.0f);
  float v_11 = 0.0f;
  if ((params.numPlanes == 1u)) {
    vec4 v_12 = textureLod(arg_0_plane0_arg_1, v_9, float(0.0f));
    v_10 = v_12.xyz;
    v_11 = v_12[3u];
  } else {
    float v_13 = textureLod(arg_0_plane0_arg_1, v_9, float(0.0f))[0u];
    vec2 v_14 = clamp(v_8, params.samplePlane1RectMin, params.samplePlane1RectMax);
    v_10 = (vec4(v_13, textureLod(arg_0_plane1_arg_1, v_14, float(0.0f)).xy, 1.0f) * params.yuvToRgbConversionMatrix);
    v_11 = 1.0f;
  }
  vec3 v_15 = v_10;
  vec3 v_16 = vec3(0.0f);
  if ((params.doYuvToRgbConversionOnly == 0u)) {
    v_16 = tint_GammaCorrection((params.gamutConversionMatrix * tint_GammaCorrection(v_15, params.gammaDecodeParams)), params.gammaEncodeParams);
  } else {
    v_16 = v_15;
  }
  return vec4(v_16, v_11);
}
tint_ExternalTextureParams tint_convert_tint_ExternalTextureParams(tint_ExternalTextureParams_std140 tint_input) {
  mat3 v_17 = mat3(tint_input.gamutConversionMatrix_col0, tint_input.gamutConversionMatrix_col1, tint_input.gamutConversionMatrix_col2);
  mat3x2 v_18 = mat3x2(tint_input.sampleTransform_col0, tint_input.sampleTransform_col1, tint_input.sampleTransform_col2);
  return tint_ExternalTextureParams(tint_input.numPlanes, tint_input.doYuvToRgbConversionOnly, tint_input.yuvToRgbConversionMatrix, tint_input.gammaDecodeParams, tint_input.gammaEncodeParams, v_17, v_18, mat3x2(tint_input.loadTransform_col0, tint_input.loadTransform_col1, tint_input.loadTransform_col2), tint_input.samplePlane0RectMin, tint_input.samplePlane0RectMax, tint_input.samplePlane1RectMin, tint_input.samplePlane1RectMax, tint_input.visibleSize, tint_input.plane1CoordFactor);
}
vec4 textureSampleBaseClampToEdge_7c04e6() {
  vec2 arg_2 = vec2(1.0f);
  tint_ExternalTextureParams v_19 = tint_convert_tint_ExternalTextureParams(v_2.inner);
  vec4 res = tint_TextureSampleExternal(v_19, arg_2);
  return res;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  v_1.inner = textureSampleBaseClampToEdge_7c04e6();
}
#version 310 es


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

struct tint_ExternalTextureParams_std140 {
  uint numPlanes;
  uint doYuvToRgbConversionOnly;
  uint tint_pad_0;
  uint tint_pad_1;
  mat3x4 yuvToRgbConversionMatrix;
  tint_GammaTransferParams gammaDecodeParams;
  tint_GammaTransferParams gammaEncodeParams;
  vec3 gamutConversionMatrix_col0;
  uint tint_pad_2;
  vec3 gamutConversionMatrix_col1;
  uint tint_pad_3;
  vec3 gamutConversionMatrix_col2;
  uint tint_pad_4;
  vec2 sampleTransform_col0;
  vec2 sampleTransform_col1;
  vec2 sampleTransform_col2;
  vec2 loadTransform_col0;
  vec2 loadTransform_col1;
  vec2 loadTransform_col2;
  vec2 samplePlane0RectMin;
  vec2 samplePlane0RectMax;
  vec2 samplePlane1RectMin;
  vec2 samplePlane1RectMax;
  uvec2 visibleSize;
  vec2 plane1CoordFactor;
};

struct tint_ExternalTextureParams {
  uint numPlanes;
  uint doYuvToRgbConversionOnly;
  mat3x4 yuvToRgbConversionMatrix;
  tint_GammaTransferParams gammaDecodeParams;
  tint_GammaTransferParams gammaEncodeParams;
  mat3 gamutConversionMatrix;
  mat3x2 sampleTransform;
  mat3x2 loadTransform;
  vec2 samplePlane0RectMin;
  vec2 samplePlane0RectMax;
  vec2 samplePlane1RectMin;
  vec2 samplePlane1RectMax;
  uvec2 visibleSize;
  vec2 plane1CoordFactor;
};

struct VertexOutput {
  vec4 pos;
  vec4 prevent_dce;
};

layout(binding = 3, std140)
uniform arg_0_params_block_std140_1_ubo {
  tint_ExternalTextureParams_std140 inner;
} v_1;
uniform highp sampler2D arg_0_plane0_arg_1;
uniform highp sampler2D arg_0_plane1_arg_1;
layout(location = 0) flat out vec4 vertex_main_loc0_Output;
vec3 tint_GammaCorrection(vec3 v, tint_GammaTransferParams params) {
  vec3 v_2 = vec3(params.G);
  vec3 v_3 = vec3(params.D);
  vec3 v_4 = abs(v);
  vec3 v_5 = sign(v);
  bvec3 v_6 = lessThan(v_4, v_3);
  return mix((v_5 * (pow(((params.A * v_4) + params.B), v_2) + params.E)), (v_5 * ((params.C * v_4) + params.F)), v_6);
}
vec4 tint_TextureSampleExternal(tint_ExternalTextureParams params, vec2 coords) {
  vec2 v_7 = (params.sampleTransform * vec3(coords, 1.0f));
  vec2 v_8 = clamp(v_7, params.samplePlane0RectMin, params.samplePlane0RectMax);
  vec3 v_9 = vec3(0.0f);
  float v_10 = 0.0f;
  if ((params.numPlanes == 1u)) {
    vec4 v_11 = textureLod(arg_0_plane0_arg_1, v_8, float(0.0f));
    v_9 = v_11.xyz;
    v_10 = v_11[3u];
  } else {
    float v_12 = textureLod(arg_0_plane0_arg_1, v_8, float(0.0f))[0u];
    vec2 v_13 = clamp(v_7, params.samplePlane1RectMin, params.samplePlane1RectMax);
    v_9 = (vec4(v_12, textureLod(arg_0_plane1_arg_1, v_13, float(0.0f)).xy, 1.0f) * params.yuvToRgbConversionMatrix);
    v_10 = 1.0f;
  }
  vec3 v_14 = v_9;
  vec3 v_15 = vec3(0.0f);
  if ((params.doYuvToRgbConversionOnly == 0u)) {
    v_15 = tint_GammaCorrection((params.gamutConversionMatrix * tint_GammaCorrection(v_14, params.gammaDecodeParams)), params.gammaEncodeParams);
  } else {
    v_15 = v_14;
  }
  return vec4(v_15, v_10);
}
tint_ExternalTextureParams tint_convert_tint_ExternalTextureParams(tint_ExternalTextureParams_std140 tint_input) {
  mat3 v_16 = mat3(tint_input.gamutConversionMatrix_col0, tint_input.gamutConversionMatrix_col1, tint_input.gamutConversionMatrix_col2);
  mat3x2 v_17 = mat3x2(tint_input.sampleTransform_col0, tint_input.sampleTransform_col1, tint_input.sampleTransform_col2);
  return tint_ExternalTextureParams(tint_input.numPlanes, tint_input.doYuvToRgbConversionOnly, tint_input.yuvToRgbConversionMatrix, tint_input.gammaDecodeParams, tint_input.gammaEncodeParams, v_16, v_17, mat3x2(tint_input.loadTransform_col0, tint_input.loadTransform_col1, tint_input.loadTransform_col2), tint_input.samplePlane0RectMin, tint_input.samplePlane0RectMax, tint_input.samplePlane1RectMin, tint_input.samplePlane1RectMax, tint_input.visibleSize, tint_input.plane1CoordFactor);
}
vec4 textureSampleBaseClampToEdge_7c04e6() {
  vec2 arg_2 = vec2(1.0f);
  tint_ExternalTextureParams v_18 = tint_convert_tint_ExternalTextureParams(v_1.inner);
  vec4 res = tint_TextureSampleExternal(v_18, arg_2);
  return res;
}
VertexOutput vertex_main_inner() {
  VertexOutput tint_symbol = VertexOutput(vec4(0.0f), vec4(0.0f));
  tint_symbol.pos = vec4(0.0f);
  tint_symbol.prevent_dce = textureSampleBaseClampToEdge_7c04e6();
  return tint_symbol;
}
void main() {
  VertexOutput v_19 = vertex_main_inner();
  gl_Position = v_19.pos;
  gl_Position[1u] = -(gl_Position.y);
  gl_Position[2u] = ((2.0f * gl_Position.z) - gl_Position.w);
  vertex_main_loc0_Output = v_19.prevent_dce;
  gl_PointSize = 1.0f;
}

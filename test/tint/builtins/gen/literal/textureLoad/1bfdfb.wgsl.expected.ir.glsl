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
  mat3x4 yuvToRgbConversionMatrix;
  tint_GammaTransferParams gammaDecodeParams;
  tint_GammaTransferParams gammaEncodeParams;
  vec3 gamutConversionMatrix_col0;
  vec3 gamutConversionMatrix_col1;
  vec3 gamutConversionMatrix_col2;
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
buffer tint_symbol_1_1_ssbo {
  vec4 tint_symbol;
} v_1;
layout(binding = 2, std140)
uniform tint_symbol_3_std140_1_ubo {
  tint_ExternalTextureParams_std140 tint_symbol_2;
} v_2;
uniform highp sampler2D arg_0_plane0;
uniform highp sampler2D arg_0_plane1;
vec3 tint_GammaCorrection(vec3 v, tint_GammaTransferParams params) {
  vec3 v_3 = vec3(params.G);
  vec3 v_4 = vec3(params.D);
  vec3 v_5 = abs(v);
  vec3 v_6 = sign(v);
  bvec3 v_7 = lessThan(v_5, v_4);
  return mix((v_6 * (pow(((params.A * v_5) + params.B), v_3) + params.E)), (v_6 * ((params.C * v_5) + params.F)), v_7);
}
vec4 tint_TextureLoadExternal(tint_ExternalTextureParams params, uvec2 coords) {
  vec2 v_8 = round((params.loadTransform * vec3(vec2(min(coords, params.visibleSize)), 1.0f)));
  uvec2 v_9 = uvec2(v_8);
  vec3 v_10 = vec3(0.0f);
  float v_11 = 0.0f;
  if ((params.numPlanes == 1u)) {
    ivec2 v_12 = ivec2(v_9);
    vec4 v_13 = texelFetch(arg_0_plane0, v_12, int(0u));
    v_10 = v_13.xyz;
    v_11 = v_13[3u];
  } else {
    ivec2 v_14 = ivec2(v_9);
    float v_15 = texelFetch(arg_0_plane0, v_14, int(0u))[0u];
    ivec2 v_16 = ivec2(uvec2((v_8 * params.plane1CoordFactor)));
    v_10 = (vec4(v_15, texelFetch(arg_0_plane1, v_16, int(0u)).xy, 1.0f) * params.yuvToRgbConversionMatrix);
    v_11 = 1.0f;
  }
  vec3 v_17 = v_10;
  vec3 v_18 = vec3(0.0f);
  if ((params.doYuvToRgbConversionOnly == 0u)) {
    v_18 = tint_GammaCorrection((params.gamutConversionMatrix * tint_GammaCorrection(v_17, params.gammaDecodeParams)), params.gammaEncodeParams);
  } else {
    v_18 = v_17;
  }
  return vec4(v_18, v_11);
}
tint_ExternalTextureParams tint_convert_tint_ExternalTextureParams(tint_ExternalTextureParams_std140 tint_input) {
  mat3 v_19 = mat3(tint_input.gamutConversionMatrix_col0, tint_input.gamutConversionMatrix_col1, tint_input.gamutConversionMatrix_col2);
  mat3x2 v_20 = mat3x2(tint_input.sampleTransform_col0, tint_input.sampleTransform_col1, tint_input.sampleTransform_col2);
  return tint_ExternalTextureParams(tint_input.numPlanes, tint_input.doYuvToRgbConversionOnly, tint_input.yuvToRgbConversionMatrix, tint_input.gammaDecodeParams, tint_input.gammaEncodeParams, v_19, v_20, mat3x2(tint_input.loadTransform_col0, tint_input.loadTransform_col1, tint_input.loadTransform_col2), tint_input.samplePlane0RectMin, tint_input.samplePlane0RectMax, tint_input.samplePlane1RectMin, tint_input.samplePlane1RectMax, tint_input.visibleSize, tint_input.plane1CoordFactor);
}
vec4 textureLoad_1bfdfb() {
  vec4 res = tint_TextureLoadExternal(tint_convert_tint_ExternalTextureParams(v_2.tint_symbol_2), uvec2(1u));
  return res;
}
void main() {
  v_1.tint_symbol = textureLoad_1bfdfb();
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
  mat3x4 yuvToRgbConversionMatrix;
  tint_GammaTransferParams gammaDecodeParams;
  tint_GammaTransferParams gammaEncodeParams;
  vec3 gamutConversionMatrix_col0;
  vec3 gamutConversionMatrix_col1;
  vec3 gamutConversionMatrix_col2;
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
buffer tint_symbol_1_1_ssbo {
  vec4 tint_symbol;
} v_1;
layout(binding = 2, std140)
uniform tint_symbol_3_std140_1_ubo {
  tint_ExternalTextureParams_std140 tint_symbol_2;
} v_2;
uniform highp sampler2D arg_0_plane0;
uniform highp sampler2D arg_0_plane1;
vec3 tint_GammaCorrection(vec3 v, tint_GammaTransferParams params) {
  vec3 v_3 = vec3(params.G);
  vec3 v_4 = vec3(params.D);
  vec3 v_5 = abs(v);
  vec3 v_6 = sign(v);
  bvec3 v_7 = lessThan(v_5, v_4);
  return mix((v_6 * (pow(((params.A * v_5) + params.B), v_3) + params.E)), (v_6 * ((params.C * v_5) + params.F)), v_7);
}
vec4 tint_TextureLoadExternal(tint_ExternalTextureParams params, uvec2 coords) {
  vec2 v_8 = round((params.loadTransform * vec3(vec2(min(coords, params.visibleSize)), 1.0f)));
  uvec2 v_9 = uvec2(v_8);
  vec3 v_10 = vec3(0.0f);
  float v_11 = 0.0f;
  if ((params.numPlanes == 1u)) {
    ivec2 v_12 = ivec2(v_9);
    vec4 v_13 = texelFetch(arg_0_plane0, v_12, int(0u));
    v_10 = v_13.xyz;
    v_11 = v_13[3u];
  } else {
    ivec2 v_14 = ivec2(v_9);
    float v_15 = texelFetch(arg_0_plane0, v_14, int(0u))[0u];
    ivec2 v_16 = ivec2(uvec2((v_8 * params.plane1CoordFactor)));
    v_10 = (vec4(v_15, texelFetch(arg_0_plane1, v_16, int(0u)).xy, 1.0f) * params.yuvToRgbConversionMatrix);
    v_11 = 1.0f;
  }
  vec3 v_17 = v_10;
  vec3 v_18 = vec3(0.0f);
  if ((params.doYuvToRgbConversionOnly == 0u)) {
    v_18 = tint_GammaCorrection((params.gamutConversionMatrix * tint_GammaCorrection(v_17, params.gammaDecodeParams)), params.gammaEncodeParams);
  } else {
    v_18 = v_17;
  }
  return vec4(v_18, v_11);
}
tint_ExternalTextureParams tint_convert_tint_ExternalTextureParams(tint_ExternalTextureParams_std140 tint_input) {
  mat3 v_19 = mat3(tint_input.gamutConversionMatrix_col0, tint_input.gamutConversionMatrix_col1, tint_input.gamutConversionMatrix_col2);
  mat3x2 v_20 = mat3x2(tint_input.sampleTransform_col0, tint_input.sampleTransform_col1, tint_input.sampleTransform_col2);
  return tint_ExternalTextureParams(tint_input.numPlanes, tint_input.doYuvToRgbConversionOnly, tint_input.yuvToRgbConversionMatrix, tint_input.gammaDecodeParams, tint_input.gammaEncodeParams, v_19, v_20, mat3x2(tint_input.loadTransform_col0, tint_input.loadTransform_col1, tint_input.loadTransform_col2), tint_input.samplePlane0RectMin, tint_input.samplePlane0RectMax, tint_input.samplePlane1RectMin, tint_input.samplePlane1RectMax, tint_input.visibleSize, tint_input.plane1CoordFactor);
}
vec4 textureLoad_1bfdfb() {
  vec4 res = tint_TextureLoadExternal(tint_convert_tint_ExternalTextureParams(v_2.tint_symbol_2), uvec2(1u));
  return res;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  v_1.tint_symbol = textureLoad_1bfdfb();
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
  mat3x4 yuvToRgbConversionMatrix;
  tint_GammaTransferParams gammaDecodeParams;
  tint_GammaTransferParams gammaEncodeParams;
  vec3 gamutConversionMatrix_col0;
  vec3 gamutConversionMatrix_col1;
  vec3 gamutConversionMatrix_col2;
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

layout(binding = 2, std140)
uniform tint_symbol_2_std140_1_ubo {
  tint_ExternalTextureParams_std140 tint_symbol_1;
} v_1;
uniform highp sampler2D arg_0_plane0;
uniform highp sampler2D arg_0_plane1;
layout(location = 0) flat out vec4 vertex_main_loc0_Output;
vec3 tint_GammaCorrection(vec3 v, tint_GammaTransferParams params) {
  vec3 v_2 = vec3(params.G);
  vec3 v_3 = vec3(params.D);
  vec3 v_4 = abs(v);
  vec3 v_5 = sign(v);
  bvec3 v_6 = lessThan(v_4, v_3);
  return mix((v_5 * (pow(((params.A * v_4) + params.B), v_2) + params.E)), (v_5 * ((params.C * v_4) + params.F)), v_6);
}
vec4 tint_TextureLoadExternal(tint_ExternalTextureParams params, uvec2 coords) {
  vec2 v_7 = round((params.loadTransform * vec3(vec2(min(coords, params.visibleSize)), 1.0f)));
  uvec2 v_8 = uvec2(v_7);
  vec3 v_9 = vec3(0.0f);
  float v_10 = 0.0f;
  if ((params.numPlanes == 1u)) {
    ivec2 v_11 = ivec2(v_8);
    vec4 v_12 = texelFetch(arg_0_plane0, v_11, int(0u));
    v_9 = v_12.xyz;
    v_10 = v_12[3u];
  } else {
    ivec2 v_13 = ivec2(v_8);
    float v_14 = texelFetch(arg_0_plane0, v_13, int(0u))[0u];
    ivec2 v_15 = ivec2(uvec2((v_7 * params.plane1CoordFactor)));
    v_9 = (vec4(v_14, texelFetch(arg_0_plane1, v_15, int(0u)).xy, 1.0f) * params.yuvToRgbConversionMatrix);
    v_10 = 1.0f;
  }
  vec3 v_16 = v_9;
  vec3 v_17 = vec3(0.0f);
  if ((params.doYuvToRgbConversionOnly == 0u)) {
    v_17 = tint_GammaCorrection((params.gamutConversionMatrix * tint_GammaCorrection(v_16, params.gammaDecodeParams)), params.gammaEncodeParams);
  } else {
    v_17 = v_16;
  }
  return vec4(v_17, v_10);
}
tint_ExternalTextureParams tint_convert_tint_ExternalTextureParams(tint_ExternalTextureParams_std140 tint_input) {
  mat3 v_18 = mat3(tint_input.gamutConversionMatrix_col0, tint_input.gamutConversionMatrix_col1, tint_input.gamutConversionMatrix_col2);
  mat3x2 v_19 = mat3x2(tint_input.sampleTransform_col0, tint_input.sampleTransform_col1, tint_input.sampleTransform_col2);
  return tint_ExternalTextureParams(tint_input.numPlanes, tint_input.doYuvToRgbConversionOnly, tint_input.yuvToRgbConversionMatrix, tint_input.gammaDecodeParams, tint_input.gammaEncodeParams, v_18, v_19, mat3x2(tint_input.loadTransform_col0, tint_input.loadTransform_col1, tint_input.loadTransform_col2), tint_input.samplePlane0RectMin, tint_input.samplePlane0RectMax, tint_input.samplePlane1RectMin, tint_input.samplePlane1RectMax, tint_input.visibleSize, tint_input.plane1CoordFactor);
}
vec4 textureLoad_1bfdfb() {
  vec4 res = tint_TextureLoadExternal(tint_convert_tint_ExternalTextureParams(v_1.tint_symbol_1), uvec2(1u));
  return res;
}
VertexOutput vertex_main_inner() {
  VertexOutput tint_symbol = VertexOutput(vec4(0.0f), vec4(0.0f));
  tint_symbol.pos = vec4(0.0f);
  tint_symbol.prevent_dce = textureLoad_1bfdfb();
  return tint_symbol;
}
void main() {
  VertexOutput v_20 = vertex_main_inner();
  gl_Position = v_20.pos;
  gl_Position[1u] = -(gl_Position.y);
  gl_Position[2u] = ((2.0f * gl_Position.z) - gl_Position.w);
  vertex_main_loc0_Output = v_20.prevent_dce;
  gl_PointSize = 1.0f;
}

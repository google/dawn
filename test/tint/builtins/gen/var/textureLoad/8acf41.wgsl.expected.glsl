#version 310 es

uvec2 tint_select(uvec2 param_0, uvec2 param_1, bvec2 param_2) {
    return uvec2(param_2[0] ? param_1[0] : param_0[0], param_2[1] ? param_1[1] : param_0[1]);
}

vec3 tint_select_1(vec3 param_0, vec3 param_1, bvec3 param_2) {
    return vec3(param_2[0] ? param_1[0] : param_0[0], param_2[1] ? param_1[1] : param_0[1], param_2[2] ? param_1[2] : param_0[2]);
}


uvec2 tint_ftou(vec2 v) {
  return tint_select(uvec2(4294967295u), tint_select(uvec2(v), uvec2(0u), lessThan(v, vec2(0.0f))), lessThan(v, vec2(4294967040.0f)));
}

struct GammaTransferParams {
  float G;
  float A;
  float B;
  float C;
  float D;
  float E;
  float F;
  uint padding;
};

struct ExternalTextureParams {
  uint numPlanes;
  uint doYuvToRgbConversionOnly;
  uint pad;
  uint pad_1;
  mat3x4 yuvToRgbConversionMatrix;
  GammaTransferParams gammaDecodeParams;
  GammaTransferParams gammaEncodeParams;
  mat3 gamutConversionMatrix;
  mat3x2 coordTransformationMatrix;
  mat3x2 loadTransformationMatrix;
  vec2 samplePlane0RectMin;
  vec2 samplePlane0RectMax;
  vec2 samplePlane1RectMin;
  vec2 samplePlane1RectMax;
  uvec2 displayVisibleRectMax;
  vec2 plane1CoordFactor;
};

struct ExternalTextureParams_std140 {
  uint numPlanes;
  uint doYuvToRgbConversionOnly;
  uint pad;
  uint pad_1;
  mat3x4 yuvToRgbConversionMatrix;
  GammaTransferParams gammaDecodeParams;
  GammaTransferParams gammaEncodeParams;
  mat3 gamutConversionMatrix;
  vec2 coordTransformationMatrix_0;
  vec2 coordTransformationMatrix_1;
  vec2 coordTransformationMatrix_2;
  vec2 loadTransformationMatrix_0;
  vec2 loadTransformationMatrix_1;
  vec2 loadTransformationMatrix_2;
  vec2 samplePlane0RectMin;
  vec2 samplePlane0RectMax;
  vec2 samplePlane1RectMin;
  vec2 samplePlane1RectMax;
  uvec2 displayVisibleRectMax;
  vec2 plane1CoordFactor;
};

layout(binding = 2, std140) uniform ext_tex_params_block_std140_ubo {
  ExternalTextureParams_std140 inner;
} ext_tex_params;

vec3 gammaCorrection(vec3 v, GammaTransferParams params) {
  bvec3 cond = lessThan(abs(v), vec3(params.D));
  vec3 t = (sign(v) * ((params.C * abs(v)) + params.F));
  vec3 f = (sign(v) * (pow(((params.A * abs(v)) + params.B), vec3(params.G)) + params.E));
  return tint_select_1(f, t, cond);
}

vec4 textureLoadExternal(highp sampler2D plane0_1, highp sampler2D plane1_1, ivec2 coord, ExternalTextureParams params) {
  uvec2 clampedCoords = min(uvec2(coord), params.displayVisibleRectMax);
  uvec2 plane0_clamped = tint_ftou(round((params.loadTransformationMatrix * vec3(vec2(clampedCoords), 1.0f))));
  vec4 color = vec4(0.0f, 0.0f, 0.0f, 0.0f);
  if ((params.numPlanes == 1u)) {
    color = texelFetch(plane0_1, ivec2(plane0_clamped), 0).rgba;
  } else {
    uvec2 plane1_clamped = tint_ftou((vec2(plane0_clamped) * params.plane1CoordFactor));
    color = vec4((vec4(texelFetch(plane0_1, ivec2(plane0_clamped), 0).r, texelFetch(plane1_1, ivec2(plane1_clamped), 0).rg, 1.0f) * params.yuvToRgbConversionMatrix), 1.0f);
  }
  if ((params.doYuvToRgbConversionOnly == 0u)) {
    color = vec4(gammaCorrection(color.rgb, params.gammaDecodeParams), color.a);
    color = vec4((params.gamutConversionMatrix * color.rgb), color.a);
    color = vec4(gammaCorrection(color.rgb, params.gammaEncodeParams), color.a);
  }
  return color;
}

uniform highp sampler2D arg_0_1;
uniform highp sampler2D ext_tex_plane_1_1;
ExternalTextureParams conv_ExternalTextureParams(ExternalTextureParams_std140 val) {
  return ExternalTextureParams(val.numPlanes, val.doYuvToRgbConversionOnly, val.pad, val.pad_1, val.yuvToRgbConversionMatrix, val.gammaDecodeParams, val.gammaEncodeParams, val.gamutConversionMatrix, mat3x2(val.coordTransformationMatrix_0, val.coordTransformationMatrix_1, val.coordTransformationMatrix_2), mat3x2(val.loadTransformationMatrix_0, val.loadTransformationMatrix_1, val.loadTransformationMatrix_2), val.samplePlane0RectMin, val.samplePlane0RectMax, val.samplePlane1RectMin, val.samplePlane1RectMax, val.displayVisibleRectMax, val.plane1CoordFactor);
}

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  vec4 inner;
} prevent_dce;

void textureLoad_8acf41() {
  ivec2 arg_1 = ivec2(1);
  vec4 res = textureLoadExternal(arg_0_1, ext_tex_plane_1_1, arg_1, conv_ExternalTextureParams(ext_tex_params.inner));
  prevent_dce.inner = res;
}

vec4 vertex_main() {
  textureLoad_8acf41();
  return vec4(0.0f);
}

void main() {
  gl_PointSize = 1.0;
  vec4 inner_result = vertex_main();
  gl_Position = inner_result;
  gl_Position.y = -(gl_Position.y);
  gl_Position.z = ((2.0f * gl_Position.z) - gl_Position.w);
  return;
}
#version 310 es
precision highp float;
precision highp int;

uvec2 tint_select(uvec2 param_0, uvec2 param_1, bvec2 param_2) {
    return uvec2(param_2[0] ? param_1[0] : param_0[0], param_2[1] ? param_1[1] : param_0[1]);
}

vec3 tint_select_1(vec3 param_0, vec3 param_1, bvec3 param_2) {
    return vec3(param_2[0] ? param_1[0] : param_0[0], param_2[1] ? param_1[1] : param_0[1], param_2[2] ? param_1[2] : param_0[2]);
}


uvec2 tint_ftou(vec2 v) {
  return tint_select(uvec2(4294967295u), tint_select(uvec2(v), uvec2(0u), lessThan(v, vec2(0.0f))), lessThan(v, vec2(4294967040.0f)));
}

struct GammaTransferParams {
  float G;
  float A;
  float B;
  float C;
  float D;
  float E;
  float F;
  uint padding;
};

struct ExternalTextureParams {
  uint numPlanes;
  uint doYuvToRgbConversionOnly;
  uint pad;
  uint pad_1;
  mat3x4 yuvToRgbConversionMatrix;
  GammaTransferParams gammaDecodeParams;
  GammaTransferParams gammaEncodeParams;
  mat3 gamutConversionMatrix;
  mat3x2 coordTransformationMatrix;
  mat3x2 loadTransformationMatrix;
  vec2 samplePlane0RectMin;
  vec2 samplePlane0RectMax;
  vec2 samplePlane1RectMin;
  vec2 samplePlane1RectMax;
  uvec2 displayVisibleRectMax;
  vec2 plane1CoordFactor;
};

struct ExternalTextureParams_std140 {
  uint numPlanes;
  uint doYuvToRgbConversionOnly;
  uint pad;
  uint pad_1;
  mat3x4 yuvToRgbConversionMatrix;
  GammaTransferParams gammaDecodeParams;
  GammaTransferParams gammaEncodeParams;
  mat3 gamutConversionMatrix;
  vec2 coordTransformationMatrix_0;
  vec2 coordTransformationMatrix_1;
  vec2 coordTransformationMatrix_2;
  vec2 loadTransformationMatrix_0;
  vec2 loadTransformationMatrix_1;
  vec2 loadTransformationMatrix_2;
  vec2 samplePlane0RectMin;
  vec2 samplePlane0RectMax;
  vec2 samplePlane1RectMin;
  vec2 samplePlane1RectMax;
  uvec2 displayVisibleRectMax;
  vec2 plane1CoordFactor;
};

layout(binding = 2, std140) uniform ext_tex_params_block_std140_ubo {
  ExternalTextureParams_std140 inner;
} ext_tex_params;

vec3 gammaCorrection(vec3 v, GammaTransferParams params) {
  bvec3 cond = lessThan(abs(v), vec3(params.D));
  vec3 t = (sign(v) * ((params.C * abs(v)) + params.F));
  vec3 f = (sign(v) * (pow(((params.A * abs(v)) + params.B), vec3(params.G)) + params.E));
  return tint_select_1(f, t, cond);
}

vec4 textureLoadExternal(highp sampler2D plane0_1, highp sampler2D plane1_1, ivec2 coord, ExternalTextureParams params) {
  uvec2 clampedCoords = min(uvec2(coord), params.displayVisibleRectMax);
  uvec2 plane0_clamped = tint_ftou(round((params.loadTransformationMatrix * vec3(vec2(clampedCoords), 1.0f))));
  vec4 color = vec4(0.0f, 0.0f, 0.0f, 0.0f);
  if ((params.numPlanes == 1u)) {
    color = texelFetch(plane0_1, ivec2(plane0_clamped), 0).rgba;
  } else {
    uvec2 plane1_clamped = tint_ftou((vec2(plane0_clamped) * params.plane1CoordFactor));
    color = vec4((vec4(texelFetch(plane0_1, ivec2(plane0_clamped), 0).r, texelFetch(plane1_1, ivec2(plane1_clamped), 0).rg, 1.0f) * params.yuvToRgbConversionMatrix), 1.0f);
  }
  if ((params.doYuvToRgbConversionOnly == 0u)) {
    color = vec4(gammaCorrection(color.rgb, params.gammaDecodeParams), color.a);
    color = vec4((params.gamutConversionMatrix * color.rgb), color.a);
    color = vec4(gammaCorrection(color.rgb, params.gammaEncodeParams), color.a);
  }
  return color;
}

uniform highp sampler2D arg_0_1;
uniform highp sampler2D ext_tex_plane_1_1;
ExternalTextureParams conv_ExternalTextureParams(ExternalTextureParams_std140 val) {
  return ExternalTextureParams(val.numPlanes, val.doYuvToRgbConversionOnly, val.pad, val.pad_1, val.yuvToRgbConversionMatrix, val.gammaDecodeParams, val.gammaEncodeParams, val.gamutConversionMatrix, mat3x2(val.coordTransformationMatrix_0, val.coordTransformationMatrix_1, val.coordTransformationMatrix_2), mat3x2(val.loadTransformationMatrix_0, val.loadTransformationMatrix_1, val.loadTransformationMatrix_2), val.samplePlane0RectMin, val.samplePlane0RectMax, val.samplePlane1RectMin, val.samplePlane1RectMax, val.displayVisibleRectMax, val.plane1CoordFactor);
}

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  vec4 inner;
} prevent_dce;

void textureLoad_8acf41() {
  ivec2 arg_1 = ivec2(1);
  vec4 res = textureLoadExternal(arg_0_1, ext_tex_plane_1_1, arg_1, conv_ExternalTextureParams(ext_tex_params.inner));
  prevent_dce.inner = res;
}

void fragment_main() {
  textureLoad_8acf41();
}

void main() {
  fragment_main();
  return;
}
#version 310 es

uvec2 tint_select(uvec2 param_0, uvec2 param_1, bvec2 param_2) {
    return uvec2(param_2[0] ? param_1[0] : param_0[0], param_2[1] ? param_1[1] : param_0[1]);
}

vec3 tint_select_1(vec3 param_0, vec3 param_1, bvec3 param_2) {
    return vec3(param_2[0] ? param_1[0] : param_0[0], param_2[1] ? param_1[1] : param_0[1], param_2[2] ? param_1[2] : param_0[2]);
}


uvec2 tint_ftou(vec2 v) {
  return tint_select(uvec2(4294967295u), tint_select(uvec2(v), uvec2(0u), lessThan(v, vec2(0.0f))), lessThan(v, vec2(4294967040.0f)));
}

struct GammaTransferParams {
  float G;
  float A;
  float B;
  float C;
  float D;
  float E;
  float F;
  uint padding;
};

struct ExternalTextureParams {
  uint numPlanes;
  uint doYuvToRgbConversionOnly;
  uint pad;
  uint pad_1;
  mat3x4 yuvToRgbConversionMatrix;
  GammaTransferParams gammaDecodeParams;
  GammaTransferParams gammaEncodeParams;
  mat3 gamutConversionMatrix;
  mat3x2 coordTransformationMatrix;
  mat3x2 loadTransformationMatrix;
  vec2 samplePlane0RectMin;
  vec2 samplePlane0RectMax;
  vec2 samplePlane1RectMin;
  vec2 samplePlane1RectMax;
  uvec2 displayVisibleRectMax;
  vec2 plane1CoordFactor;
};

struct ExternalTextureParams_std140 {
  uint numPlanes;
  uint doYuvToRgbConversionOnly;
  uint pad;
  uint pad_1;
  mat3x4 yuvToRgbConversionMatrix;
  GammaTransferParams gammaDecodeParams;
  GammaTransferParams gammaEncodeParams;
  mat3 gamutConversionMatrix;
  vec2 coordTransformationMatrix_0;
  vec2 coordTransformationMatrix_1;
  vec2 coordTransformationMatrix_2;
  vec2 loadTransformationMatrix_0;
  vec2 loadTransformationMatrix_1;
  vec2 loadTransformationMatrix_2;
  vec2 samplePlane0RectMin;
  vec2 samplePlane0RectMax;
  vec2 samplePlane1RectMin;
  vec2 samplePlane1RectMax;
  uvec2 displayVisibleRectMax;
  vec2 plane1CoordFactor;
};

layout(binding = 2, std140) uniform ext_tex_params_block_std140_ubo {
  ExternalTextureParams_std140 inner;
} ext_tex_params;

vec3 gammaCorrection(vec3 v, GammaTransferParams params) {
  bvec3 cond = lessThan(abs(v), vec3(params.D));
  vec3 t = (sign(v) * ((params.C * abs(v)) + params.F));
  vec3 f = (sign(v) * (pow(((params.A * abs(v)) + params.B), vec3(params.G)) + params.E));
  return tint_select_1(f, t, cond);
}

vec4 textureLoadExternal(highp sampler2D plane0_1, highp sampler2D plane1_1, ivec2 coord, ExternalTextureParams params) {
  uvec2 clampedCoords = min(uvec2(coord), params.displayVisibleRectMax);
  uvec2 plane0_clamped = tint_ftou(round((params.loadTransformationMatrix * vec3(vec2(clampedCoords), 1.0f))));
  vec4 color = vec4(0.0f, 0.0f, 0.0f, 0.0f);
  if ((params.numPlanes == 1u)) {
    color = texelFetch(plane0_1, ivec2(plane0_clamped), 0).rgba;
  } else {
    uvec2 plane1_clamped = tint_ftou((vec2(plane0_clamped) * params.plane1CoordFactor));
    color = vec4((vec4(texelFetch(plane0_1, ivec2(plane0_clamped), 0).r, texelFetch(plane1_1, ivec2(plane1_clamped), 0).rg, 1.0f) * params.yuvToRgbConversionMatrix), 1.0f);
  }
  if ((params.doYuvToRgbConversionOnly == 0u)) {
    color = vec4(gammaCorrection(color.rgb, params.gammaDecodeParams), color.a);
    color = vec4((params.gamutConversionMatrix * color.rgb), color.a);
    color = vec4(gammaCorrection(color.rgb, params.gammaEncodeParams), color.a);
  }
  return color;
}

uniform highp sampler2D arg_0_1;
uniform highp sampler2D ext_tex_plane_1_1;
ExternalTextureParams conv_ExternalTextureParams(ExternalTextureParams_std140 val) {
  return ExternalTextureParams(val.numPlanes, val.doYuvToRgbConversionOnly, val.pad, val.pad_1, val.yuvToRgbConversionMatrix, val.gammaDecodeParams, val.gammaEncodeParams, val.gamutConversionMatrix, mat3x2(val.coordTransformationMatrix_0, val.coordTransformationMatrix_1, val.coordTransformationMatrix_2), mat3x2(val.loadTransformationMatrix_0, val.loadTransformationMatrix_1, val.loadTransformationMatrix_2), val.samplePlane0RectMin, val.samplePlane0RectMax, val.samplePlane1RectMin, val.samplePlane1RectMax, val.displayVisibleRectMax, val.plane1CoordFactor);
}

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  vec4 inner;
} prevent_dce;

void textureLoad_8acf41() {
  ivec2 arg_1 = ivec2(1);
  vec4 res = textureLoadExternal(arg_0_1, ext_tex_plane_1_1, arg_1, conv_ExternalTextureParams(ext_tex_params.inner));
  prevent_dce.inner = res;
}

void compute_main() {
  textureLoad_8acf41();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main();
  return;
}

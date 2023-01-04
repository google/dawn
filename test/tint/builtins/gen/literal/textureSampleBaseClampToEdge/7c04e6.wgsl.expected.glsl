#version 310 es

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
  mat3x4 yuvToRgbConversionMatrix;
  GammaTransferParams gammaDecodeParams;
  GammaTransferParams gammaEncodeParams;
  mat3 gamutConversionMatrix;
  mat3x2 coordTransformationMatrix;
};

struct ExternalTextureParams_std140 {
  uint numPlanes;
  uint doYuvToRgbConversionOnly;
  mat3x4 yuvToRgbConversionMatrix;
  GammaTransferParams gammaDecodeParams;
  GammaTransferParams gammaEncodeParams;
  mat3 gamutConversionMatrix;
  vec2 coordTransformationMatrix_0;
  vec2 coordTransformationMatrix_1;
  vec2 coordTransformationMatrix_2;
};

layout(binding = 3, std140) uniform ext_tex_params_block_std140_ubo {
  ExternalTextureParams_std140 inner;
} ext_tex_params;

vec3 gammaCorrection(vec3 v, GammaTransferParams params) {
  bvec3 cond = lessThan(abs(v), vec3(params.D));
  vec3 t = (sign(v) * ((params.C * abs(v)) + params.F));
  vec3 f = (sign(v) * (pow(((params.A * abs(v)) + params.B), vec3(params.G)) + params.E));
  return mix(f, t, cond);
}


vec4 textureSampleExternal(highp sampler2D plane0_1, highp sampler2D plane1_1, highp sampler2D plane0_smp, highp sampler2D plane1_smp, vec2 coord, ExternalTextureParams params) {
  vec2 modifiedCoords = (params.coordTransformationMatrix * vec3(coord, 1.0f));
  vec2 plane0_dims = vec2(uvec2(textureSize(plane0_1, 0)));
  vec2 plane0_half_texel = (vec2(0.5f) / plane0_dims);
  vec2 plane0_clamped = clamp(modifiedCoords, plane0_half_texel, (1.0f - plane0_half_texel));
  vec2 plane1_dims = vec2(uvec2(textureSize(plane1_1, 0)));
  vec2 plane1_half_texel = (vec2(0.5f) / plane1_dims);
  vec2 plane1_clamped = clamp(modifiedCoords, plane1_half_texel, (1.0f - plane1_half_texel));
  vec3 color = vec3(0.0f, 0.0f, 0.0f);
  if ((params.numPlanes == 1u)) {
    color = textureLod(plane0_smp, plane0_clamped, 0.0f).rgb;
  } else {
    color = (vec4(textureLod(plane0_smp, plane0_clamped, 0.0f).r, textureLod(plane1_smp, plane1_clamped, 0.0f).rg, 1.0f) * params.yuvToRgbConversionMatrix);
  }
  if ((params.doYuvToRgbConversionOnly == 0u)) {
    color = gammaCorrection(color, params.gammaDecodeParams);
    color = (params.gamutConversionMatrix * color);
    color = gammaCorrection(color, params.gammaEncodeParams);
  }
  return vec4(color, 1.0f);
}

uniform highp sampler2D arg_0_1;
uniform highp sampler2D ext_tex_plane_1_1;
uniform highp sampler2D arg_0_arg_1;
uniform highp sampler2D ext_tex_plane_1_arg_1;
ExternalTextureParams conv_ExternalTextureParams(ExternalTextureParams_std140 val) {
  return ExternalTextureParams(val.numPlanes, val.doYuvToRgbConversionOnly, val.yuvToRgbConversionMatrix, val.gammaDecodeParams, val.gammaEncodeParams, val.gamutConversionMatrix, mat3x2(val.coordTransformationMatrix_0, val.coordTransformationMatrix_1, val.coordTransformationMatrix_2));
}

void textureSampleBaseClampToEdge_7c04e6() {
  vec4 res = textureSampleExternal(arg_0_1, ext_tex_plane_1_1, arg_0_arg_1, ext_tex_plane_1_arg_1, vec2(1.0f), conv_ExternalTextureParams(ext_tex_params.inner));
}

vec4 vertex_main() {
  textureSampleBaseClampToEdge_7c04e6();
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
precision mediump float;

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
  mat3x4 yuvToRgbConversionMatrix;
  GammaTransferParams gammaDecodeParams;
  GammaTransferParams gammaEncodeParams;
  mat3 gamutConversionMatrix;
  mat3x2 coordTransformationMatrix;
};

struct ExternalTextureParams_std140 {
  uint numPlanes;
  uint doYuvToRgbConversionOnly;
  mat3x4 yuvToRgbConversionMatrix;
  GammaTransferParams gammaDecodeParams;
  GammaTransferParams gammaEncodeParams;
  mat3 gamutConversionMatrix;
  vec2 coordTransformationMatrix_0;
  vec2 coordTransformationMatrix_1;
  vec2 coordTransformationMatrix_2;
};

layout(binding = 3, std140) uniform ext_tex_params_block_std140_ubo {
  ExternalTextureParams_std140 inner;
} ext_tex_params;

vec3 gammaCorrection(vec3 v, GammaTransferParams params) {
  bvec3 cond = lessThan(abs(v), vec3(params.D));
  vec3 t = (sign(v) * ((params.C * abs(v)) + params.F));
  vec3 f = (sign(v) * (pow(((params.A * abs(v)) + params.B), vec3(params.G)) + params.E));
  return mix(f, t, cond);
}


vec4 textureSampleExternal(highp sampler2D plane0_1, highp sampler2D plane1_1, highp sampler2D plane0_smp, highp sampler2D plane1_smp, vec2 coord, ExternalTextureParams params) {
  vec2 modifiedCoords = (params.coordTransformationMatrix * vec3(coord, 1.0f));
  vec2 plane0_dims = vec2(uvec2(textureSize(plane0_1, 0)));
  vec2 plane0_half_texel = (vec2(0.5f) / plane0_dims);
  vec2 plane0_clamped = clamp(modifiedCoords, plane0_half_texel, (1.0f - plane0_half_texel));
  vec2 plane1_dims = vec2(uvec2(textureSize(plane1_1, 0)));
  vec2 plane1_half_texel = (vec2(0.5f) / plane1_dims);
  vec2 plane1_clamped = clamp(modifiedCoords, plane1_half_texel, (1.0f - plane1_half_texel));
  vec3 color = vec3(0.0f, 0.0f, 0.0f);
  if ((params.numPlanes == 1u)) {
    color = textureLod(plane0_smp, plane0_clamped, 0.0f).rgb;
  } else {
    color = (vec4(textureLod(plane0_smp, plane0_clamped, 0.0f).r, textureLod(plane1_smp, plane1_clamped, 0.0f).rg, 1.0f) * params.yuvToRgbConversionMatrix);
  }
  if ((params.doYuvToRgbConversionOnly == 0u)) {
    color = gammaCorrection(color, params.gammaDecodeParams);
    color = (params.gamutConversionMatrix * color);
    color = gammaCorrection(color, params.gammaEncodeParams);
  }
  return vec4(color, 1.0f);
}

uniform highp sampler2D arg_0_1;
uniform highp sampler2D ext_tex_plane_1_1;
uniform highp sampler2D arg_0_arg_1;
uniform highp sampler2D ext_tex_plane_1_arg_1;
ExternalTextureParams conv_ExternalTextureParams(ExternalTextureParams_std140 val) {
  return ExternalTextureParams(val.numPlanes, val.doYuvToRgbConversionOnly, val.yuvToRgbConversionMatrix, val.gammaDecodeParams, val.gammaEncodeParams, val.gamutConversionMatrix, mat3x2(val.coordTransformationMatrix_0, val.coordTransformationMatrix_1, val.coordTransformationMatrix_2));
}

void textureSampleBaseClampToEdge_7c04e6() {
  vec4 res = textureSampleExternal(arg_0_1, ext_tex_plane_1_1, arg_0_arg_1, ext_tex_plane_1_arg_1, vec2(1.0f), conv_ExternalTextureParams(ext_tex_params.inner));
}

void fragment_main() {
  textureSampleBaseClampToEdge_7c04e6();
}

void main() {
  fragment_main();
  return;
}
#version 310 es

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
  mat3x4 yuvToRgbConversionMatrix;
  GammaTransferParams gammaDecodeParams;
  GammaTransferParams gammaEncodeParams;
  mat3 gamutConversionMatrix;
  mat3x2 coordTransformationMatrix;
};

struct ExternalTextureParams_std140 {
  uint numPlanes;
  uint doYuvToRgbConversionOnly;
  mat3x4 yuvToRgbConversionMatrix;
  GammaTransferParams gammaDecodeParams;
  GammaTransferParams gammaEncodeParams;
  mat3 gamutConversionMatrix;
  vec2 coordTransformationMatrix_0;
  vec2 coordTransformationMatrix_1;
  vec2 coordTransformationMatrix_2;
};

layout(binding = 3, std140) uniform ext_tex_params_block_std140_ubo {
  ExternalTextureParams_std140 inner;
} ext_tex_params;

vec3 gammaCorrection(vec3 v, GammaTransferParams params) {
  bvec3 cond = lessThan(abs(v), vec3(params.D));
  vec3 t = (sign(v) * ((params.C * abs(v)) + params.F));
  vec3 f = (sign(v) * (pow(((params.A * abs(v)) + params.B), vec3(params.G)) + params.E));
  return mix(f, t, cond);
}


vec4 textureSampleExternal(highp sampler2D plane0_1, highp sampler2D plane1_1, highp sampler2D plane0_smp, highp sampler2D plane1_smp, vec2 coord, ExternalTextureParams params) {
  vec2 modifiedCoords = (params.coordTransformationMatrix * vec3(coord, 1.0f));
  vec2 plane0_dims = vec2(uvec2(textureSize(plane0_1, 0)));
  vec2 plane0_half_texel = (vec2(0.5f) / plane0_dims);
  vec2 plane0_clamped = clamp(modifiedCoords, plane0_half_texel, (1.0f - plane0_half_texel));
  vec2 plane1_dims = vec2(uvec2(textureSize(plane1_1, 0)));
  vec2 plane1_half_texel = (vec2(0.5f) / plane1_dims);
  vec2 plane1_clamped = clamp(modifiedCoords, plane1_half_texel, (1.0f - plane1_half_texel));
  vec3 color = vec3(0.0f, 0.0f, 0.0f);
  if ((params.numPlanes == 1u)) {
    color = textureLod(plane0_smp, plane0_clamped, 0.0f).rgb;
  } else {
    color = (vec4(textureLod(plane0_smp, plane0_clamped, 0.0f).r, textureLod(plane1_smp, plane1_clamped, 0.0f).rg, 1.0f) * params.yuvToRgbConversionMatrix);
  }
  if ((params.doYuvToRgbConversionOnly == 0u)) {
    color = gammaCorrection(color, params.gammaDecodeParams);
    color = (params.gamutConversionMatrix * color);
    color = gammaCorrection(color, params.gammaEncodeParams);
  }
  return vec4(color, 1.0f);
}

uniform highp sampler2D arg_0_1;
uniform highp sampler2D ext_tex_plane_1_1;
uniform highp sampler2D arg_0_arg_1;
uniform highp sampler2D ext_tex_plane_1_arg_1;
ExternalTextureParams conv_ExternalTextureParams(ExternalTextureParams_std140 val) {
  return ExternalTextureParams(val.numPlanes, val.doYuvToRgbConversionOnly, val.yuvToRgbConversionMatrix, val.gammaDecodeParams, val.gammaEncodeParams, val.gamutConversionMatrix, mat3x2(val.coordTransformationMatrix_0, val.coordTransformationMatrix_1, val.coordTransformationMatrix_2));
}

void textureSampleBaseClampToEdge_7c04e6() {
  vec4 res = textureSampleExternal(arg_0_1, ext_tex_plane_1_1, arg_0_arg_1, ext_tex_plane_1_arg_1, vec2(1.0f), conv_ExternalTextureParams(ext_tex_params.inner));
}

void compute_main() {
  textureSampleBaseClampToEdge_7c04e6();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main();
  return;
}

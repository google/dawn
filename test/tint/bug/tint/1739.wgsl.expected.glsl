SKIP: FAILED

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
  uint pad;
  uint pad_1;
  mat3x4 yuvToRgbConversionMatrix;
  GammaTransferParams gammaDecodeParams;
  GammaTransferParams gammaEncodeParams;
  mat3 gamutConversionMatrix;
  mat3x2 coordTransformationMatrix;
  uint pad_2;
  uint pad_3;
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
  uint pad_2;
  uint pad_3;
};

layout(binding = 3, std140) uniform ext_tex_params_block_std140_ubo {
  ExternalTextureParams_std140 inner;
} ext_tex_params;

layout(rgba8) uniform highp writeonly image2D outImage;
vec3 gammaCorrection(vec3 v, GammaTransferParams params) {
  bvec3 cond = lessThan(abs(v), vec3(params.D));
  vec3 t_1 = (sign(v) * ((params.C * abs(v)) + params.F));
  vec3 f = (sign(v) * (pow(((params.A * abs(v)) + params.B), vec3(params.G)) + params.E));
  return mix(f, t_1, cond);
}

vec4 textureLoadExternal(highp sampler2D plane0_1, highp sampler2D plane1_1, ivec2 coord, ExternalTextureParams params) {
  ivec2 coord1 = (coord >> uvec2(1u));
  vec3 color = vec3(0.0f, 0.0f, 0.0f);
  if ((params.numPlanes == 1u)) {
    color = texelFetch(plane0_1, clamp(coord, ivec2(0), ivec2((uvec2(uvec2(textureSize(plane0_1, clamp(0, 0, int((uint(uint(textureQueryLevels(plane0_1))) - 1u)))))) - uvec2(1u)))), clamp(0, 0, int((uint(uint(textureQueryLevels(plane0_1))) - 1u)))).rgb;
  } else {
    color = (vec4(texelFetch(plane0_1, clamp(coord, ivec2(0), ivec2((uvec2(uvec2(textureSize(plane0_1, clamp(0, 0, int((uint(uint(textureQueryLevels(plane0_1))) - 1u)))))) - uvec2(1u)))), clamp(0, 0, int((uint(uint(textureQueryLevels(plane0_1))) - 1u)))).r, texelFetch(plane1_1, clamp(coord1, ivec2(0), ivec2((uvec2(uvec2(textureSize(plane1_1, clamp(0, 0, int((uint(uint(textureQueryLevels(plane1_1))) - 1u)))))) - uvec2(1u)))), clamp(0, 0, int((uint(uint(textureQueryLevels(plane1_1))) - 1u)))).rg, 1.0f) * params.yuvToRgbConversionMatrix);
  }
  if ((params.doYuvToRgbConversionOnly == 0u)) {
    color = gammaCorrection(color, params.gammaDecodeParams);
    color = (params.gamutConversionMatrix * color);
    color = gammaCorrection(color, params.gammaEncodeParams);
  }
  return vec4(color, 1.0f);
}

uniform highp sampler2D t_2;
uniform highp sampler2D ext_tex_plane_1_1;
ExternalTextureParams conv_ExternalTextureParams(ExternalTextureParams_std140 val) {
  return ExternalTextureParams(val.numPlanes, val.doYuvToRgbConversionOnly, val.pad, val.pad_1, val.yuvToRgbConversionMatrix, val.gammaDecodeParams, val.gammaEncodeParams, val.gamutConversionMatrix, mat3x2(val.coordTransformationMatrix_0, val.coordTransformationMatrix_1, val.coordTransformationMatrix_2), val.pad_2, val.pad_3);
}

void tint_symbol() {
  vec4 red = textureLoadExternal(t_2, ext_tex_plane_1_1, ivec2(10), conv_ExternalTextureParams(ext_tex_params.inner));
  imageStore(outImage, clamp(ivec2(0), ivec2(0), ivec2((uvec2(uvec2(imageSize(outImage))) - uvec2(1u)))), red);
  vec4 green = textureLoadExternal(t_2, ext_tex_plane_1_1, ivec2(70, 118), conv_ExternalTextureParams(ext_tex_params.inner));
  imageStore(outImage, clamp(ivec2(1, 0), ivec2(0), ivec2((uvec2(uvec2(imageSize(outImage))) - uvec2(1u)))), green);
  return;
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  tint_symbol();
  return;
}
Error parsing GLSL shader:
ERROR: 0:60: 'textureQueryLevels' : no matching overloaded function found 
ERROR: 0:60: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




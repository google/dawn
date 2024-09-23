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

uniform highp sampler2D t_plane0;
uniform highp sampler2D t_plane1;
layout(binding = 2, std140)
uniform tint_symbol_1_std140_1_ubo {
  tint_ExternalTextureParams_std140 tint_symbol;
} v_1;
vec3 tint_GammaCorrection(vec3 v, tint_GammaTransferParams params) {
  vec3 v_2 = vec3(params.G);
  vec3 v_3 = vec3(params.D);
  vec3 v_4 = abs(v);
  vec3 v_5 = sign(v);
  bvec3 v_6 = lessThan(v_4, v_3);
  vec3 v_7 = (v_5 * (pow(((params.A * v_4) + params.B), v_2) + params.E));
  float v_8 = ((v_6.x) ? ((v_5 * ((params.C * v_4) + params.F)).x) : (v_7.x));
  float v_9 = ((v_6.y) ? ((v_5 * ((params.C * v_4) + params.F)).y) : (v_7.y));
  return vec3(v_8, v_9, ((v_6.z) ? ((v_5 * ((params.C * v_4) + params.F)).z) : (v_7.z)));
}
vec4 tint_TextureLoadExternal(highp sampler2D plane_0, highp sampler2D plane_1, tint_ExternalTextureParams params, uvec2 coords) {
  vec2 v_10 = round((params.loadTransform * vec3(vec2(min(coords, params.visibleSize)), 1.0f)));
  uvec2 v_11 = uvec2(v_10);
  vec3 v_12 = vec3(0.0f);
  float v_13 = 0.0f;
  if ((params.numPlanes == 1u)) {
    ivec2 v_14 = ivec2(v_11);
    vec4 v_15 = texelFetch(plane_0, v_14, int(0u));
    v_12 = v_15.xyz;
    v_13 = v_15[3u];
  } else {
    ivec2 v_16 = ivec2(v_11);
    float v_17 = texelFetch(plane_0, v_16, int(0u))[0u];
    ivec2 v_18 = ivec2(uvec2((v_10 * params.plane1CoordFactor)));
    v_12 = (vec4(v_17, texelFetch(plane_1, v_18, int(0u)).xy, 1.0f) * params.yuvToRgbConversionMatrix);
    v_13 = 1.0f;
  }
  vec3 v_19 = v_12;
  vec3 v_20 = vec3(0.0f);
  if ((params.doYuvToRgbConversionOnly == 0u)) {
    v_20 = tint_GammaCorrection((params.gamutConversionMatrix * tint_GammaCorrection(v_19, params.gammaDecodeParams)), params.gammaEncodeParams);
  } else {
    v_20 = v_19;
  }
  return vec4(v_20, v_13);
}
tint_ExternalTextureParams tint_convert_tint_ExternalTextureParams(tint_ExternalTextureParams_std140 tint_input) {
  mat3 v_21 = mat3(tint_input.gamutConversionMatrix_col0, tint_input.gamutConversionMatrix_col1, tint_input.gamutConversionMatrix_col2);
  mat3x2 v_22 = mat3x2(tint_input.sampleTransform_col0, tint_input.sampleTransform_col1, tint_input.sampleTransform_col2);
  return tint_ExternalTextureParams(tint_input.numPlanes, tint_input.doYuvToRgbConversionOnly, tint_input.yuvToRgbConversionMatrix, tint_input.gammaDecodeParams, tint_input.gammaEncodeParams, v_21, v_22, mat3x2(tint_input.loadTransform_col0, tint_input.loadTransform_col1, tint_input.loadTransform_col2), tint_input.samplePlane0RectMin, tint_input.samplePlane0RectMax, tint_input.samplePlane1RectMin, tint_input.samplePlane1RectMax, tint_input.visibleSize, tint_input.plane1CoordFactor);
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  tint_ExternalTextureParams v_23 = tint_convert_tint_ExternalTextureParams(v_1.tint_symbol);
  vec4 r = tint_TextureLoadExternal(t_plane0, t_plane1, v_23, uvec2(ivec2(0)));
}

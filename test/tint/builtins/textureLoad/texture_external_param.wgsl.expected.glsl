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
  mat3x4 yuvToRgbConversionMatrix;
  GammaTransferParams gammaDecodeParams;
  GammaTransferParams gammaEncodeParams;
  mat3 gamutConversionMatrix;
};

layout(binding = 2) uniform ExternalTextureParams_1 {
  uint numPlanes;
  uint doYuvToRgbConversionOnly;
  mat3x4 yuvToRgbConversionMatrix;
  GammaTransferParams gammaDecodeParams;
  GammaTransferParams gammaEncodeParams;
  mat3 gamutConversionMatrix;
} ext_tex_params;

vec3 gammaCorrection(vec3 v, GammaTransferParams params) {
  bvec3 cond = lessThan(abs(v), vec3(params.D));
  vec3 t = (sign(v) * ((params.C * abs(v)) + params.F));
  vec3 f = (sign(v) * (pow(((params.A * abs(v)) + params.B), vec3(params.G)) + params.E));
  return mix(f, t, cond);
}

vec4 textureLoadExternal(highp sampler2D plane0_1, highp sampler2D plane1_1, ivec2 coord, ExternalTextureParams params) {
  vec3 color = vec3(0.0f, 0.0f, 0.0f);
  if ((params.numPlanes == 1u)) {
    color = texelFetch(plane0_1, coord, 0).rgb;
  } else {
    color = (vec4(texelFetch(plane0_1, coord, 0).r, texelFetch(plane1_1, coord, 0).rg, 1.0f) * params.yuvToRgbConversionMatrix);
  }
  if ((params.doYuvToRgbConversionOnly == 0u)) {
    color = gammaCorrection(color, params.gammaDecodeParams);
    color = (params.gamutConversionMatrix * color);
    color = gammaCorrection(color, params.gammaEncodeParams);
  }
  return vec4(color, 1.0f);
}

vec4 textureLoad2d(highp sampler2D tint_symbol_1, highp sampler2D ext_tex_plane_1_1_1, ExternalTextureParams ext_tex_params_1, ivec2 coords) {
  return textureLoadExternal(tint_symbol_1, ext_tex_plane_1_1_1, coords, ext_tex_params_1);
}

uniform highp sampler2D arg_0_1;
uniform highp sampler2D ext_tex_plane_1_2;
void doTextureLoad() {
  vec4 res = textureLoad2d(arg_0_1, ext_tex_plane_1_2, ext_tex_params, ivec2(0));
}

vec4 vertex_main() {
  doTextureLoad();
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
Error parsing GLSL shader:
ERROR: 0:61: 'textureLoad2d' : no matching overloaded function found 
ERROR: 0:61: '=' :  cannot convert from ' const float' to ' temp highp 4-component vector of float'
ERROR: 0:61: '' : compilation terminated 
ERROR: 3 compilation errors.  No code generated.



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
};

layout(binding = 2) uniform ExternalTextureParams_1 {
  uint numPlanes;
  uint doYuvToRgbConversionOnly;
  mat3x4 yuvToRgbConversionMatrix;
  GammaTransferParams gammaDecodeParams;
  GammaTransferParams gammaEncodeParams;
  mat3 gamutConversionMatrix;
} ext_tex_params;

vec3 gammaCorrection(vec3 v, GammaTransferParams params) {
  bvec3 cond = lessThan(abs(v), vec3(params.D));
  vec3 t = (sign(v) * ((params.C * abs(v)) + params.F));
  vec3 f = (sign(v) * (pow(((params.A * abs(v)) + params.B), vec3(params.G)) + params.E));
  return mix(f, t, cond);
}

vec4 textureLoadExternal(highp sampler2D plane0_1, highp sampler2D plane1_1, ivec2 coord, ExternalTextureParams params) {
  vec3 color = vec3(0.0f, 0.0f, 0.0f);
  if ((params.numPlanes == 1u)) {
    color = texelFetch(plane0_1, coord, 0).rgb;
  } else {
    color = (vec4(texelFetch(plane0_1, coord, 0).r, texelFetch(plane1_1, coord, 0).rg, 1.0f) * params.yuvToRgbConversionMatrix);
  }
  if ((params.doYuvToRgbConversionOnly == 0u)) {
    color = gammaCorrection(color, params.gammaDecodeParams);
    color = (params.gamutConversionMatrix * color);
    color = gammaCorrection(color, params.gammaEncodeParams);
  }
  return vec4(color, 1.0f);
}

vec4 textureLoad2d(highp sampler2D tint_symbol_1, highp sampler2D ext_tex_plane_1_1_1, ExternalTextureParams ext_tex_params_1, ivec2 coords) {
  return textureLoadExternal(tint_symbol_1, ext_tex_plane_1_1_1, coords, ext_tex_params_1);
}

uniform highp sampler2D arg_0_1;
uniform highp sampler2D ext_tex_plane_1_2;
void doTextureLoad() {
  vec4 res = textureLoad2d(arg_0_1, ext_tex_plane_1_2, ext_tex_params, ivec2(0));
}

void fragment_main() {
  doTextureLoad();
}

void main() {
  fragment_main();
  return;
}
Error parsing GLSL shader:
ERROR: 0:62: 'textureLoad2d' : no matching overloaded function found 
ERROR: 0:62: '=' :  cannot convert from ' const float' to ' temp mediump 4-component vector of float'
ERROR: 0:62: '' : compilation terminated 
ERROR: 3 compilation errors.  No code generated.



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
};

layout(binding = 2) uniform ExternalTextureParams_1 {
  uint numPlanes;
  uint doYuvToRgbConversionOnly;
  mat3x4 yuvToRgbConversionMatrix;
  GammaTransferParams gammaDecodeParams;
  GammaTransferParams gammaEncodeParams;
  mat3 gamutConversionMatrix;
} ext_tex_params;

vec3 gammaCorrection(vec3 v, GammaTransferParams params) {
  bvec3 cond = lessThan(abs(v), vec3(params.D));
  vec3 t = (sign(v) * ((params.C * abs(v)) + params.F));
  vec3 f = (sign(v) * (pow(((params.A * abs(v)) + params.B), vec3(params.G)) + params.E));
  return mix(f, t, cond);
}

vec4 textureLoadExternal(highp sampler2D plane0_1, highp sampler2D plane1_1, ivec2 coord, ExternalTextureParams params) {
  vec3 color = vec3(0.0f, 0.0f, 0.0f);
  if ((params.numPlanes == 1u)) {
    color = texelFetch(plane0_1, coord, 0).rgb;
  } else {
    color = (vec4(texelFetch(plane0_1, coord, 0).r, texelFetch(plane1_1, coord, 0).rg, 1.0f) * params.yuvToRgbConversionMatrix);
  }
  if ((params.doYuvToRgbConversionOnly == 0u)) {
    color = gammaCorrection(color, params.gammaDecodeParams);
    color = (params.gamutConversionMatrix * color);
    color = gammaCorrection(color, params.gammaEncodeParams);
  }
  return vec4(color, 1.0f);
}

vec4 textureLoad2d(highp sampler2D tint_symbol_1, highp sampler2D ext_tex_plane_1_1_1, ExternalTextureParams ext_tex_params_1, ivec2 coords) {
  return textureLoadExternal(tint_symbol_1, ext_tex_plane_1_1_1, coords, ext_tex_params_1);
}

uniform highp sampler2D arg_0_1;
uniform highp sampler2D ext_tex_plane_1_2;
void doTextureLoad() {
  vec4 res = textureLoad2d(arg_0_1, ext_tex_plane_1_2, ext_tex_params, ivec2(0));
}

void compute_main() {
  doTextureLoad();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main();
  return;
}
Error parsing GLSL shader:
ERROR: 0:61: 'textureLoad2d' : no matching overloaded function found 
ERROR: 0:61: '=' :  cannot convert from ' const float' to ' temp highp 4-component vector of float'
ERROR: 0:61: '' : compilation terminated 
ERROR: 3 compilation errors.  No code generated.




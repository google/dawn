SKIP: FAILED

#version 310 es

struct ExternalTextureParams {
  uint numPlanes;
  float vr;
  float ug;
  float vg;
  float ub;
};

layout(binding = 2) uniform ExternalTextureParams_1 {
  uint numPlanes;
  float vr;
  float ug;
  float vg;
  float ub;
} ext_tex_params;

vec4 textureLoadExternal(highp sampler2D plane0_1, highp sampler2D plane1_1, ivec2 coord, ExternalTextureParams params) {
  if ((params.numPlanes == 1u)) {
    return texelFetch(plane0_1, coord, 0);
  }
  float y = (texelFetch(plane0_1, coord, 0).r - 0.0625f);
  vec2 uv = (texelFetch(plane1_1, coord, 0).rg - 0.5f);
  float u = uv.x;
  float v = uv.y;
  float r = ((1.164000034f * y) + (params.vr * v));
  float g = (((1.164000034f * y) - (params.ug * u)) - (params.vg * v));
  float b = ((1.164000034f * y) + (params.ub * u));
  return vec4(r, g, b, 1.0f);
}

vec4 textureLoad2d(highp sampler2D tint_symbol_1, highp sampler2D ext_tex_plane_1_1_1, ExternalTextureParams ext_tex_params_1, ivec2 coords) {
  return textureLoadExternal(tint_symbol_1, ext_tex_plane_1_1_1, coords, ext_tex_params_1);
}

uniform highp sampler2D arg_0_1;
uniform highp sampler2D ext_tex_plane_1_2;
void doTextureLoad() {
  vec4 res = textureLoad2d(arg_0_1, ext_tex_plane_1_2, ext_tex_params, ivec2(0, 0));
}

vec4 vertex_main() {
  doTextureLoad();
  return vec4(0.0f, 0.0f, 0.0f, 0.0f);
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
ERROR: 0:40: 'textureLoad2d' : no matching overloaded function found 
ERROR: 0:40: '=' :  cannot convert from ' const float' to ' temp highp 4-component vector of float'
ERROR: 0:40: '' : compilation terminated 
ERROR: 3 compilation errors.  No code generated.



#version 310 es
precision mediump float;

struct ExternalTextureParams {
  uint numPlanes;
  float vr;
  float ug;
  float vg;
  float ub;
};

layout(binding = 2) uniform ExternalTextureParams_1 {
  uint numPlanes;
  float vr;
  float ug;
  float vg;
  float ub;
} ext_tex_params;

vec4 textureLoadExternal(highp sampler2D plane0_1, highp sampler2D plane1_1, ivec2 coord, ExternalTextureParams params) {
  if ((params.numPlanes == 1u)) {
    return texelFetch(plane0_1, coord, 0);
  }
  float y = (texelFetch(plane0_1, coord, 0).r - 0.0625f);
  vec2 uv = (texelFetch(plane1_1, coord, 0).rg - 0.5f);
  float u = uv.x;
  float v = uv.y;
  float r = ((1.164000034f * y) + (params.vr * v));
  float g = (((1.164000034f * y) - (params.ug * u)) - (params.vg * v));
  float b = ((1.164000034f * y) + (params.ub * u));
  return vec4(r, g, b, 1.0f);
}

vec4 textureLoad2d(highp sampler2D tint_symbol_1, highp sampler2D ext_tex_plane_1_1_1, ExternalTextureParams ext_tex_params_1, ivec2 coords) {
  return textureLoadExternal(tint_symbol_1, ext_tex_plane_1_1_1, coords, ext_tex_params_1);
}

uniform highp sampler2D arg_0_1;
uniform highp sampler2D ext_tex_plane_1_2;
void doTextureLoad() {
  vec4 res = textureLoad2d(arg_0_1, ext_tex_plane_1_2, ext_tex_params, ivec2(0, 0));
}

void fragment_main() {
  doTextureLoad();
}

void main() {
  fragment_main();
  return;
}
Error parsing GLSL shader:
ERROR: 0:41: 'textureLoad2d' : no matching overloaded function found 
ERROR: 0:41: '=' :  cannot convert from ' const float' to ' temp mediump 4-component vector of float'
ERROR: 0:41: '' : compilation terminated 
ERROR: 3 compilation errors.  No code generated.



#version 310 es

struct ExternalTextureParams {
  uint numPlanes;
  float vr;
  float ug;
  float vg;
  float ub;
};

layout(binding = 2) uniform ExternalTextureParams_1 {
  uint numPlanes;
  float vr;
  float ug;
  float vg;
  float ub;
} ext_tex_params;

vec4 textureLoadExternal(highp sampler2D plane0_1, highp sampler2D plane1_1, ivec2 coord, ExternalTextureParams params) {
  if ((params.numPlanes == 1u)) {
    return texelFetch(plane0_1, coord, 0);
  }
  float y = (texelFetch(plane0_1, coord, 0).r - 0.0625f);
  vec2 uv = (texelFetch(plane1_1, coord, 0).rg - 0.5f);
  float u = uv.x;
  float v = uv.y;
  float r = ((1.164000034f * y) + (params.vr * v));
  float g = (((1.164000034f * y) - (params.ug * u)) - (params.vg * v));
  float b = ((1.164000034f * y) + (params.ub * u));
  return vec4(r, g, b, 1.0f);
}

vec4 textureLoad2d(highp sampler2D tint_symbol_1, highp sampler2D ext_tex_plane_1_1_1, ExternalTextureParams ext_tex_params_1, ivec2 coords) {
  return textureLoadExternal(tint_symbol_1, ext_tex_plane_1_1_1, coords, ext_tex_params_1);
}

uniform highp sampler2D arg_0_1;
uniform highp sampler2D ext_tex_plane_1_2;
void doTextureLoad() {
  vec4 res = textureLoad2d(arg_0_1, ext_tex_plane_1_2, ext_tex_params, ivec2(0, 0));
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
ERROR: 0:40: 'textureLoad2d' : no matching overloaded function found 
ERROR: 0:40: '=' :  cannot convert from ' const float' to ' temp highp 4-component vector of float'
ERROR: 0:40: '' : compilation terminated 
ERROR: 3 compilation errors.  No code generated.




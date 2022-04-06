SKIP: FAILED

#version 310 es

struct ExternalTextureParams {
  uint numPlanes;
  float vr;
  float ug;
  float vg;
  float ub;
};

layout(binding = 3) uniform ExternalTextureParams_1 {
  uint numPlanes;
  float vr;
  float ug;
  float vg;
  float ub;
} ext_tex_params;


vec4 textureSampleExternal(highp sampler2D plane0_smp, highp sampler2D plane1_smp, vec2 coord, ExternalTextureParams params) {
  if ((params.numPlanes == 1u)) {
    return textureLod(plane0_smp, coord, 0.0f);
  }
  float y = (textureLod(plane0_smp, coord, 0.0f).r - 0.0625f);
  vec2 uv = (textureLod(plane1_smp, coord, 0.0f).rg - 0.5f);
  float u = uv.x;
  float v = uv.y;
  float r = ((1.164000034f * y) + (params.vr * v));
  float g = (((1.164000034f * y) - (params.ug * u)) - (params.vg * v));
  float b = ((1.164000034f * y) + (params.ub * u));
  return vec4(r, g, b, 1.0f);
}

uniform highp sampler2D arg_0_arg_1;
uniform highp sampler2D ext_tex_plane_1_arg_1;
void textureSampleLevel_979816() {
  vec4 res = textureSampleExternal(arg_0_arg_1, ext_tex_plane_1_arg_1, vec2(0.0f, 0.0f), ext_tex_params);
}

vec4 vertex_main() {
  textureSampleLevel_979816();
  return vec4(0.0f, 0.0f, 0.0f, 0.0f);
}

void main() {
  vec4 inner_result = vertex_main();
  gl_Position = inner_result;
  gl_Position.y = -(gl_Position.y);
  gl_Position.z = ((2.0f * gl_Position.z) - gl_Position.w);
  return;
}
Error parsing GLSL shader:
ERROR: 0:37: 'textureSampleExternal' : no matching overloaded function found 
ERROR: 0:37: '=' :  cannot convert from ' const float' to ' temp highp 4-component vector of float'
ERROR: 0:37: '' : compilation terminated 
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

layout(binding = 3) uniform ExternalTextureParams_1 {
  uint numPlanes;
  float vr;
  float ug;
  float vg;
  float ub;
} ext_tex_params;


vec4 textureSampleExternal(highp sampler2D plane0_smp, highp sampler2D plane1_smp, vec2 coord, ExternalTextureParams params) {
  if ((params.numPlanes == 1u)) {
    return textureLod(plane0_smp, coord, 0.0f);
  }
  float y = (textureLod(plane0_smp, coord, 0.0f).r - 0.0625f);
  vec2 uv = (textureLod(plane1_smp, coord, 0.0f).rg - 0.5f);
  float u = uv.x;
  float v = uv.y;
  float r = ((1.164000034f * y) + (params.vr * v));
  float g = (((1.164000034f * y) - (params.ug * u)) - (params.vg * v));
  float b = ((1.164000034f * y) + (params.ub * u));
  return vec4(r, g, b, 1.0f);
}

uniform highp sampler2D arg_0_arg_1;
uniform highp sampler2D ext_tex_plane_1_arg_1;
void textureSampleLevel_979816() {
  vec4 res = textureSampleExternal(arg_0_arg_1, ext_tex_plane_1_arg_1, vec2(0.0f, 0.0f), ext_tex_params);
}

void fragment_main() {
  textureSampleLevel_979816();
}

void main() {
  fragment_main();
  return;
}
Error parsing GLSL shader:
ERROR: 0:38: 'textureSampleExternal' : no matching overloaded function found 
ERROR: 0:38: '=' :  cannot convert from ' const float' to ' temp mediump 4-component vector of float'
ERROR: 0:38: '' : compilation terminated 
ERROR: 3 compilation errors.  No code generated.



#version 310 es

struct ExternalTextureParams {
  uint numPlanes;
  float vr;
  float ug;
  float vg;
  float ub;
};

layout(binding = 3) uniform ExternalTextureParams_1 {
  uint numPlanes;
  float vr;
  float ug;
  float vg;
  float ub;
} ext_tex_params;


vec4 textureSampleExternal(highp sampler2D plane0_smp, highp sampler2D plane1_smp, vec2 coord, ExternalTextureParams params) {
  if ((params.numPlanes == 1u)) {
    return textureLod(plane0_smp, coord, 0.0f);
  }
  float y = (textureLod(plane0_smp, coord, 0.0f).r - 0.0625f);
  vec2 uv = (textureLod(plane1_smp, coord, 0.0f).rg - 0.5f);
  float u = uv.x;
  float v = uv.y;
  float r = ((1.164000034f * y) + (params.vr * v));
  float g = (((1.164000034f * y) - (params.ug * u)) - (params.vg * v));
  float b = ((1.164000034f * y) + (params.ub * u));
  return vec4(r, g, b, 1.0f);
}

uniform highp sampler2D arg_0_arg_1;
uniform highp sampler2D ext_tex_plane_1_arg_1;
void textureSampleLevel_979816() {
  vec4 res = textureSampleExternal(arg_0_arg_1, ext_tex_plane_1_arg_1, vec2(0.0f, 0.0f), ext_tex_params);
}

void compute_main() {
  textureSampleLevel_979816();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main();
  return;
}
Error parsing GLSL shader:
ERROR: 0:37: 'textureSampleExternal' : no matching overloaded function found 
ERROR: 0:37: '=' :  cannot convert from ' const float' to ' temp highp 4-component vector of float'
ERROR: 0:37: '' : compilation terminated 
ERROR: 3 compilation errors.  No code generated.




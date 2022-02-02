SKIP: FAILED

#version 310 es
precision mediump float;

vec2 tint_unpack2x16unorm(uint param_0) {
  uint j = param_0;
  uint2 i = uint2(j & 0xffff, j >> 16);
  return float2(i) / 65535.0;
}


void unpack2x16unorm_7699c0() {
  vec2 res = tint_unpack2x16unorm(1u);
}

vec4 vertex_main() {
  unpack2x16unorm_7699c0();
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
ERROR: 0:6: 'uint2' : undeclared identifier 
ERROR: 0:6: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



#version 310 es
precision mediump float;

vec2 tint_unpack2x16unorm(uint param_0) {
  uint j = param_0;
  uint2 i = uint2(j & 0xffff, j >> 16);
  return float2(i) / 65535.0;
}


void unpack2x16unorm_7699c0() {
  vec2 res = tint_unpack2x16unorm(1u);
}

void fragment_main() {
  unpack2x16unorm_7699c0();
}

void main() {
  fragment_main();
  return;
}
Error parsing GLSL shader:
ERROR: 0:6: 'uint2' : undeclared identifier 
ERROR: 0:6: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



#version 310 es
precision mediump float;

vec2 tint_unpack2x16unorm(uint param_0) {
  uint j = param_0;
  uint2 i = uint2(j & 0xffff, j >> 16);
  return float2(i) / 65535.0;
}


void unpack2x16unorm_7699c0() {
  vec2 res = tint_unpack2x16unorm(1u);
}

void compute_main() {
  unpack2x16unorm_7699c0();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main();
  return;
}
Error parsing GLSL shader:
ERROR: 0:6: 'uint2' : undeclared identifier 
ERROR: 0:6: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




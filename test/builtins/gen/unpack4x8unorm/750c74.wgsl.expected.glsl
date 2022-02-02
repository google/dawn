SKIP: FAILED

#version 310 es
precision mediump float;

vec4 tint_unpack4x8unorm(uint param_0) {
  uint j = param_0;
  uint4 i = uint4(j & 0xff, (j >> 8) & 0xff, (j >> 16) & 0xff, j >> 24);
  return float4(i) / 255.0;
}


void unpack4x8unorm_750c74() {
  vec4 res = tint_unpack4x8unorm(1u);
}

vec4 vertex_main() {
  unpack4x8unorm_750c74();
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
ERROR: 0:6: 'uint4' : undeclared identifier 
ERROR: 0:6: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



#version 310 es
precision mediump float;

vec4 tint_unpack4x8unorm(uint param_0) {
  uint j = param_0;
  uint4 i = uint4(j & 0xff, (j >> 8) & 0xff, (j >> 16) & 0xff, j >> 24);
  return float4(i) / 255.0;
}


void unpack4x8unorm_750c74() {
  vec4 res = tint_unpack4x8unorm(1u);
}

void fragment_main() {
  unpack4x8unorm_750c74();
}

void main() {
  fragment_main();
  return;
}
Error parsing GLSL shader:
ERROR: 0:6: 'uint4' : undeclared identifier 
ERROR: 0:6: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



#version 310 es
precision mediump float;

vec4 tint_unpack4x8unorm(uint param_0) {
  uint j = param_0;
  uint4 i = uint4(j & 0xff, (j >> 8) & 0xff, (j >> 16) & 0xff, j >> 24);
  return float4(i) / 255.0;
}


void unpack4x8unorm_750c74() {
  vec4 res = tint_unpack4x8unorm(1u);
}

void compute_main() {
  unpack4x8unorm_750c74();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main();
  return;
}
Error parsing GLSL shader:
ERROR: 0:6: 'uint4' : undeclared identifier 
ERROR: 0:6: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




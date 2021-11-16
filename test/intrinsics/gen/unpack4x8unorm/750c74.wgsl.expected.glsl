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

struct tint_symbol {
  vec4 value;
};

vec4 vertex_main_inner() {
  unpack4x8unorm_750c74();
  return vec4(0.0f, 0.0f, 0.0f, 0.0f);
}

tint_symbol vertex_main() {
  vec4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = tint_symbol(vec4(0.0f, 0.0f, 0.0f, 0.0f));
  wrapper_result.value = inner_result;
  return wrapper_result;
}
void main() {
  tint_symbol outputs;
  outputs = vertex_main();
  gl_Position = outputs.value;
  gl_Position.y = -gl_Position.y;
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

struct tint_symbol {
  vec4 value;
};

void fragment_main() {
  unpack4x8unorm_750c74();
  return;
}
void main() {
  fragment_main();
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

struct tint_symbol {
  vec4 value;
};

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void compute_main() {
  unpack4x8unorm_750c74();
  return;
}
void main() {
  compute_main();
}


Error parsing GLSL shader:
ERROR: 0:6: 'uint4' : undeclared identifier 
ERROR: 0:6: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




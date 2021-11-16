SKIP: FAILED

#version 310 es
precision mediump float;

vec2 tint_unpack2x16snorm(uint param_0) {
  int j = int(param_0);
  int2 i = int2(j << 16, j) >> 16;
  return clamp(float2(i) / 32767.0, -1.0, 1.0);
}


void unpack2x16snorm_b4aea6() {
  vec2 res = tint_unpack2x16snorm(1u);
}

struct tint_symbol {
  vec4 value;
};

vec4 vertex_main_inner() {
  unpack2x16snorm_b4aea6();
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
ERROR: 0:6: 'int2' : undeclared identifier 
ERROR: 0:6: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



#version 310 es
precision mediump float;

vec2 tint_unpack2x16snorm(uint param_0) {
  int j = int(param_0);
  int2 i = int2(j << 16, j) >> 16;
  return clamp(float2(i) / 32767.0, -1.0, 1.0);
}


void unpack2x16snorm_b4aea6() {
  vec2 res = tint_unpack2x16snorm(1u);
}

struct tint_symbol {
  vec4 value;
};

void fragment_main() {
  unpack2x16snorm_b4aea6();
  return;
}
void main() {
  fragment_main();
}


Error parsing GLSL shader:
ERROR: 0:6: 'int2' : undeclared identifier 
ERROR: 0:6: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



#version 310 es
precision mediump float;

vec2 tint_unpack2x16snorm(uint param_0) {
  int j = int(param_0);
  int2 i = int2(j << 16, j) >> 16;
  return clamp(float2(i) / 32767.0, -1.0, 1.0);
}


void unpack2x16snorm_b4aea6() {
  vec2 res = tint_unpack2x16snorm(1u);
}

struct tint_symbol {
  vec4 value;
};

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void compute_main() {
  unpack2x16snorm_b4aea6();
  return;
}
void main() {
  compute_main();
}


Error parsing GLSL shader:
ERROR: 0:6: 'int2' : undeclared identifier 
ERROR: 0:6: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




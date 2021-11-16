SKIP: FAILED

#version 310 es
precision mediump float;

bvec3 tint_isNormal(vec3 param_0) {
  uint3 exponent = asuint(param_0) & 0x7f80000;
  uint3 clamped = clamp(exponent, 0x0080000, 0x7f00000);
  return clamped == exponent;
}


void isNormal_c286b7() {
  bvec3 res = tint_isNormal(vec3(0.0f, 0.0f, 0.0f));
}

struct tint_symbol {
  vec4 value;
};

vec4 vertex_main_inner() {
  isNormal_c286b7();
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
ERROR: 0:5: 'uint3' : undeclared identifier 
ERROR: 0:5: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



#version 310 es
precision mediump float;

bvec3 tint_isNormal(vec3 param_0) {
  uint3 exponent = asuint(param_0) & 0x7f80000;
  uint3 clamped = clamp(exponent, 0x0080000, 0x7f00000);
  return clamped == exponent;
}


void isNormal_c286b7() {
  bvec3 res = tint_isNormal(vec3(0.0f, 0.0f, 0.0f));
}

struct tint_symbol {
  vec4 value;
};

void fragment_main() {
  isNormal_c286b7();
  return;
}
void main() {
  fragment_main();
}


Error parsing GLSL shader:
ERROR: 0:5: 'uint3' : undeclared identifier 
ERROR: 0:5: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



#version 310 es
precision mediump float;

bvec3 tint_isNormal(vec3 param_0) {
  uint3 exponent = asuint(param_0) & 0x7f80000;
  uint3 clamped = clamp(exponent, 0x0080000, 0x7f00000);
  return clamped == exponent;
}


void isNormal_c286b7() {
  bvec3 res = tint_isNormal(vec3(0.0f, 0.0f, 0.0f));
}

struct tint_symbol {
  vec4 value;
};

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void compute_main() {
  isNormal_c286b7();
  return;
}
void main() {
  compute_main();
}


Error parsing GLSL shader:
ERROR: 0:5: 'uint3' : undeclared identifier 
ERROR: 0:5: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




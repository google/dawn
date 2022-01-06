SKIP: FAILED

#version 310 es
precision mediump float;

struct S {
  int field0[];
};

uvec3 x_3 = uvec3(0u, 0u, 0u);
layout (binding = 0) buffer S_1 {
  int field0[];
} x_6;
layout (binding = 1) buffer S_2 {
  int field0[];
} x_7;
layout (binding = 2) buffer S_3 {
  int field0[];
} x_8;
layout (binding = 3) buffer S_4 {
  int field0[];
} x_9;

void main_1() {
  uint x_26 = x_3.x;
  int x_28 = x_6.field0[x_26];
  int x_30 = x_7.field0[x_26];
  int x_32 = x_8.field0[x_26];
  x_9.field0[x_26] = int(clamp(uint(x_28), uint(x_30), uint(x_32)));
  return;
}

struct tint_symbol_2 {
  uvec3 x_3_param;
};

void tint_symbol_inner(uvec3 x_3_param) {
  x_3 = x_3_param;
  main_1();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void tint_symbol(tint_symbol_2 tint_symbol_1) {
  tint_symbol_inner(tint_symbol_1.x_3_param);
  return;
}
void main() {
  tint_symbol_2 inputs;
  inputs.x_3_param = gl_GlobalInvocationID;
  tint_symbol(inputs);
}


Error parsing GLSL shader:
ERROR: 0:5: '' : array size required 
ERROR: 0:6: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




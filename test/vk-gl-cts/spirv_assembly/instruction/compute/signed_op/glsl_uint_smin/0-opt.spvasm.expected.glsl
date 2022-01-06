SKIP: FAILED

#version 310 es
precision mediump float;

struct S {
  uint field0[];
};

uvec3 x_3 = uvec3(0u, 0u, 0u);
layout (binding = 0) buffer S_1 {
  uint field0[];
} x_6;
layout (binding = 1) buffer S_2 {
  uint field0[];
} x_7;
layout (binding = 2) buffer S_3 {
  uint field0[];
} x_8;

void main_1() {
  uint x_21 = x_3.x;
  uint x_23 = x_6.field0[x_21];
  uint x_25 = x_7.field0[x_21];
  x_8.field0[x_21] = uint(min(int(x_23), int(x_25)));
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




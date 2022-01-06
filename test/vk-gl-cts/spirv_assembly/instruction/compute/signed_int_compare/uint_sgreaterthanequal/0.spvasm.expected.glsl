SKIP: FAILED

#version 310 es
precision mediump float;

struct S {
  uint field0[];
};

uvec3 x_2 = uvec3(0u, 0u, 0u);
layout (binding = 0) buffer S_1 {
  uint field0[];
} x_5;
layout (binding = 1) buffer S_2 {
  uint field0[];
} x_6;
layout (binding = 2) buffer S_3 {
  uint field0[];
} x_7;

void main_1() {
  uint x_21 = x_2.x;
  uint x_23 = x_5.field0[x_21];
  uint x_25 = x_6.field0[x_21];
  x_7.field0[x_21] = ((int(x_23) >= int(x_25)) ? 1u : 0u);
  return;
}

struct tint_symbol_2 {
  uvec3 x_2_param;
};

void tint_symbol_inner(uvec3 x_2_param) {
  x_2 = x_2_param;
  main_1();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void tint_symbol(tint_symbol_2 tint_symbol_1) {
  tint_symbol_inner(tint_symbol_1.x_2_param);
  return;
}
void main() {
  tint_symbol_2 inputs;
  inputs.x_2_param = gl_GlobalInvocationID;
  tint_symbol(inputs);
}


Error parsing GLSL shader:
ERROR: 0:5: '' : array size required 
ERROR: 0:6: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




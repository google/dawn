#version 310 es

uvec3 x_2 = uvec3(0u, 0u, 0u);
layout(binding = 0, std430) buffer S_1 {
  uint field0[];
} x_5;
layout(binding = 1, std430) buffer S_2 {
  uint field0[];
} x_6;
layout(binding = 2, std430) buffer S_3 {
  uint field0[];
} x_7;
void main_1() {
  uint x_21 = x_2.x;
  uint x_23 = x_5.field0[x_21];
  uint x_25 = x_6.field0[x_21];
  x_7.field0[x_21] = ((int(x_23) < int(x_25)) ? 1u : 0u);
  return;
}

void tint_symbol(uvec3 x_2_param) {
  x_2 = x_2_param;
  main_1();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  tint_symbol(gl_GlobalInvocationID);
  return;
}

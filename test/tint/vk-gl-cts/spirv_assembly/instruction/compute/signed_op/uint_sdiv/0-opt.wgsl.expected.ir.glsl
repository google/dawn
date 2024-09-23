SKIP: FAILED

#version 310 es

uvec3 x_2 = uvec3(0u);
layout(binding = 0, std430)
buffer S_1_ssbo {
  uint field0[];
} x_5;
layout(binding = 1, std430)
buffer S_2_ssbo {
  uint field0[];
} x_6;
layout(binding = 2, std430)
buffer S_3_ssbo {
  uint field0[];
} x_7;
int tint_div_i32(int lhs, int rhs) {
  return (lhs / ((((rhs == 0) | ((lhs == (-2147483647 - 1)) & (rhs == -1)))) ? (1) : (rhs)));
}
void main_1() {
  uint x_20 = x_2.x;
  uint x_22 = x_5.field0[x_20];
  uint x_24 = x_6.field0[x_20];
  int v = int(x_22);
  x_7.field0[x_20] = uint(tint_div_i32(v, int(x_24)));
}
void tint_symbol_inner(uvec3 x_2_param) {
  x_2 = x_2_param;
  main_1();
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  tint_symbol_inner(gl_GlobalInvocationID);
}
error: Error parsing GLSL shader:
ERROR: 0:17: '&' :  wrong operand types: no operation '&' exists that takes a left-hand operand of type ' temp bool' and a right operand of type ' temp bool' (or there is no acceptable conversion)
ERROR: 0:17: '|' :  wrong operand types: no operation '|' exists that takes a left-hand operand of type ' temp bool' and a right operand of type ' temp bool' (or there is no acceptable conversion)
ERROR: 0:17: '' : compilation terminated 
ERROR: 3 compilation errors.  No code generated.




tint executable returned error: exit status 1

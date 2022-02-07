SKIP: FAILED

#version 310 es

layout(location = 0) in vec3 x_2_param_1;
layout(location = 1) flat in int x_3_param_1;
layout(location = 0) flat out int x_4_1_1;
vec3 x_2 = vec3(0.0f, 0.0f, 0.0f);
int x_3 = 0;
int x_4 = 0;
vec4 tint_symbol = vec4(0.0f, 0.0f, 0.0f, 0.0f);
void main_1() {
  tint_symbol = vec4(x_2, 1.0f);
  x_4 = x_3;
  return;
}

struct main_out {
  int x_4_1;
  vec4 tint_symbol;
};

main_out tint_symbol_1(vec3 x_2_param, int x_3_param) {
  x_2 = x_2_param;
  x_3 = x_3_param;
  main_1();
  main_out tint_symbol_2 = main_out(x_4, tint_symbol);
  return tint_symbol_2;
}

void main() {
  main_out inner_result = tint_symbol_1(x_2_param_1, x_3_param_1);
  x_4_1_1 = inner_result.x_4_1;
  gl_Position = inner_result.tint_symbol;
  gl_Position.y = -(gl_Position.y);
  gl_Position.z = ((2.0f * gl_Position.z) - gl_Position.w);
  return;
}
Error parsing GLSL shader:
ERROR: 0:4: '' : vertex input cannot be further qualified 
ERROR: 0:4: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




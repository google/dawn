SKIP: FAILED

#version 310 es

layout(location = 1) flat in uint x_1_param_1;
layout(location = 2) flat in uvec2 x_2_param_1;
layout(location = 3) flat in int x_3_param_1;
layout(location = 4) flat in ivec2 x_4_param_1;
layout(location = 5) flat in float x_5_param_1;
layout(location = 6) flat in vec2 x_6_param_1;
uint x_1 = 0u;
uvec2 x_2 = uvec2(0u, 0u);
int x_3 = 0;
ivec2 x_4 = ivec2(0, 0);
float x_5 = 0.0f;
vec2 x_6 = vec2(0.0f, 0.0f);
vec4 x_8 = vec4(0.0f, 0.0f, 0.0f, 0.0f);
void main_1() {
  return;
}

struct main_out {
  vec4 x_8_1;
};

main_out tint_symbol(uint x_1_param, uvec2 x_2_param, int x_3_param, ivec2 x_4_param, float x_5_param, vec2 x_6_param) {
  x_1 = x_1_param;
  x_2 = x_2_param;
  x_3 = x_3_param;
  x_4 = x_4_param;
  x_5 = x_5_param;
  x_6 = x_6_param;
  main_1();
  main_out tint_symbol_1 = main_out(x_8);
  return tint_symbol_1;
}

void main() {
  main_out inner_result = tint_symbol(x_1_param_1, x_2_param_1, x_3_param_1, x_4_param_1, x_5_param_1, x_6_param_1);
  gl_Position = inner_result.x_8_1;
  gl_Position.y = -(gl_Position.y);
  gl_Position.z = ((2.0f * gl_Position.z) - gl_Position.w);
  return;
}
Error parsing GLSL shader:
ERROR: 0:3: '' : vertex input cannot be further qualified 
ERROR: 0:3: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




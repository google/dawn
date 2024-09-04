SKIP: FAILED

#version 310 es

struct main_out {
  uint x_1_1;
  uvec2 x_2_1;
  int x_3_1;
  ivec2 x_4_1;
  float x_5_1;
  vec2 x_6_1;
  vec4 x_8_1;
};

uint x_1 = 0u;
uvec2 x_2 = uvec2(0u);
int x_3 = 0;
ivec2 x_4 = ivec2(0);
float x_5 = 0.0f;
vec2 x_6 = vec2(0.0f);
vec4 x_8 = vec4(0.0f);
void main_1() {
}
main_out main() {
  main_1();
  return main_out(x_1, x_2, x_3, x_4, x_5, x_6, x_8);
}
error: Error parsing GLSL shader:
ERROR: 0:22: 'structure' :  entry point cannot return a value
ERROR: 0:22: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1

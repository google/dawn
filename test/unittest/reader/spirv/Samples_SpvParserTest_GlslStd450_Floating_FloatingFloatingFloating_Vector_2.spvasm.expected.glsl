SKIP: FAILED

#version 310 es
precision mediump float;

void main_1() {
  uint u1 = 10u;
  uint u2 = 15u;
  uint u3 = 20u;
  int i1 = 30;
  int i2 = 35;
  int i3 = 40;
  float f1 = 50.0f;
  float f2 = 60.0f;
  float f3 = 70.0f;
  uvec2 v2u1 = uvec2(10u, 20u);
  uvec2 v2u2 = uvec2(20u, 10u);
  uvec2 v2u3 = uvec2(15u, 15u);
  ivec2 v2i1 = ivec2(30, 40);
  ivec2 v2i2 = ivec2(40, 30);
  ivec2 v2i3 = ivec2(35, 35);
  vec2 v2f1 = vec2(50.0f, 60.0f);
  vec2 v2f2 = vec2(60.0f, 50.0f);
  vec2 v2f3 = vec2(70.0f, 70.0f);
  vec3 v3f1 = vec3(50.0f, 60.0f, 70.0f);
  vec3 v3f2 = vec3(60.0f, 70.0f, 50.0f);
  vec4 v4f1 = vec4(50.0f, 50.0f, 50.0f, 50.0f);
  vec4 v4f2 = v4f1;
  vec2 x_1 = mad(v2f1, v2f2, v2f3);
  return;
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void tint_symbol() {
  main_1();
  return;
}
void main() {
  tint_symbol();
}


Error parsing GLSL shader:
ERROR: 0:27: 'mad' : no matching overloaded function found 
ERROR: 0:27: '=' :  cannot convert from ' const float' to ' temp mediump 2-component vector of float'
ERROR: 0:27: '' : compilation terminated 
ERROR: 3 compilation errors.  No code generated.




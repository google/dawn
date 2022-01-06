SKIP: FAILED

#version 310 es
precision mediump float;

struct S {
  vec2 field0;
  uint field1;
  int field2;
};

void main_1() {
  uvec2 x_1 = uvec2(3u, 4u);
  uint x_10 = x_1[3u];
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
ERROR: 0:12: '[' :  vector index out of range '3'
ERROR: 0:12: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




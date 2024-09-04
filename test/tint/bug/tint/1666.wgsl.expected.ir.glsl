SKIP: FAILED

#version 310 es

float rarr[];
void vector() {
  int idx = 3;
  int x = ivec2(1, 2)[idx];
}
void matrix() {
  int idx = 4;
  vec2 x = mat2(vec2(1.0f, 2.0f), vec2(3.0f, 4.0f))[idx];
}
void fixed_size_array() {
  int arr[2] = int[2](1, 2);
  int idx = 3;
  int x = arr[idx];
}
void runtime_size_array() {
  int idx = -1;
  float x = rarr[idx];
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  vector();
  matrix();
  fixed_size_array();
  runtime_size_array();
}
error: Error parsing GLSL shader:
ERROR: 0:3: '' : array size required 
ERROR: 1 compilation errors.  No code generated.




tint executable returned error: exit status 1

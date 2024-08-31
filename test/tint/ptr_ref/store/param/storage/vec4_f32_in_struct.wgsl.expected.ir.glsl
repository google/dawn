#version 310 es

struct str {
  vec4 i;
};

str S;
void func() {
  S.i = vec4(0.0f);
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  func();
}

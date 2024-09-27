#version 310 es

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void unused_entry_point() {
  return;
}
int tint_div(int lhs, int rhs) {
  return (lhs / mix(rhs, 1, bool(uint((rhs == 0)) | uint(bool(uint((lhs == (-2147483647 - 1))) & uint((rhs == -1)))))));
}

void foo() {
  int a = 0;
  vec4 b = vec4(0.0f, 0.0f, 0.0f, 0.0f);
  mat2 c = mat2(0.0f, 0.0f, 0.0f, 0.0f);
  a = tint_div(a, 2);
  b = (b * mat4(vec4(0.0f), vec4(0.0f), vec4(0.0f), vec4(0.0f)));
  c = (c * 2.0f);
}


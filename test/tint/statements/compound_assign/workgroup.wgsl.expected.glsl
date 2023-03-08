#version 310 es

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void unused_entry_point() {
  return;
}
shared int a;
shared vec4 b;
shared mat2 c;
int tint_div(int lhs, int rhs) {
  return (lhs / (bool(uint((rhs == 0)) | uint(bool(uint((lhs == (-2147483647 - 1))) & uint((rhs == -1))))) ? 1 : rhs));
}

void foo() {
  a = tint_div(a, 2);
  b = (b * mat4(vec4(0.0f), vec4(0.0f), vec4(0.0f), vec4(0.0f)));
  c = (c * 2.0f);
}


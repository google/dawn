#version 310 es

void f() {
  vec2 a = vec2(2.003662109375f, -513.03125f);
  vec2 b = a;
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  f();
  return;
}

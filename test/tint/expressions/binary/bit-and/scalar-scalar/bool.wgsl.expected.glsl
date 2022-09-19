#version 310 es

void f() {
  bool a = true;
  bool b = false;
  bool r = bool(uint(a) & uint(b));
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  f();
  return;
}

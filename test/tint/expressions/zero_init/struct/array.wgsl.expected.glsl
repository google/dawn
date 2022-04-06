#version 310 es

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void unused_entry_point() {
  return;
}
struct S {
  float a[4];
};

void f() {
  S v = S(float[4](0.0f, 0.0f, 0.0f, 0.0f));
}


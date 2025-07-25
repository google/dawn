#version 310 es

void deref() {
  ivec3 a = ivec3(0);
  uint v = uint(a.x);
  a.x = int((v + uint(42)));
}
void no_deref() {
  ivec3 a = ivec3(0);
  uint v_1 = uint(a.x);
  a.x = int((v_1 + uint(42)));
}
void deref_inc() {
  ivec3 a = ivec3(0);
  uint v_2 = uint(a.x);
  a.x = int((v_2 + uint(1)));
}
void no_deref_inc() {
  ivec3 a = ivec3(0);
  uint v_3 = uint(a.x);
  a.x = int((v_3 + uint(1)));
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  deref();
  no_deref();
  deref_inc();
  no_deref_inc();
}

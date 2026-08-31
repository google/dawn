#version 310 es

void deref() {
  ivec3 a = ivec3(0);
  a.x = int((uint(a.x) + 42u));
}
void no_deref() {
  ivec3 a = ivec3(0);
  a.x = int((uint(a.x) + 42u));
}
void deref_inc() {
  ivec3 a = ivec3(0);
  a.x = int((uint(a.x) + 1u));
}
void no_deref_inc() {
  ivec3 a = ivec3(0);
  a.x = int((uint(a.x) + 1u));
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  deref();
  no_deref();
  deref_inc();
  no_deref_inc();
}

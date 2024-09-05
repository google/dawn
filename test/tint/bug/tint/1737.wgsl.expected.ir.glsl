#version 310 es

shared float a[10];
shared float b[20];
void f() {
  float x = a[0];
  float y = b[0];
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}

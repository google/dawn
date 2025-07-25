//
// main1
//
#version 310 es

int a = 0;
void uses_a() {
  uint v = uint(a);
  a = int((v + uint(1)));
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  a = 42;
  uses_a();
}
//
// main2
//
#version 310 es

int b = 0;
void uses_b() {
  uint v = uint(b);
  b = int((v * uint(2)));
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  b = 7;
  uses_b();
}
//
// main3
//
#version 310 es

int a = 0;
int b = 0;
void uses_a() {
  uint v = uint(a);
  a = int((v + uint(1)));
}
void uses_b() {
  uint v_1 = uint(b);
  b = int((v_1 * uint(2)));
}
void uses_a_and_b() {
  b = a;
}
void no_uses() {
}
void outer() {
  a = 0;
  uses_a();
  uses_a_and_b();
  uses_b();
  no_uses();
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  outer();
  no_uses();
}
//
// main4
//
#version 310 es

void no_uses() {
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  no_uses();
}

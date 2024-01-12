#version 310 es

uniform int a;
void uses_a() {
  int foo = a;
}

void main1() {
  uses_a();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  main1();
  return;
}
#version 310 es

uniform int a;
void uses_a() {
  int foo = a;
}

void uses_uses_a() {
  uses_a();
}

void main2() {
  uses_uses_a();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  main2();
  return;
}
#version 310 es

uniform int b;
void uses_b() {
  int foo = b;
}

void main3() {
  uses_b();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  main3();
  return;
}
#version 310 es

void main4() {
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  main4();
  return;
}

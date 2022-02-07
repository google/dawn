#version 310 es

int a = 0;
void uses_a() {
  a = (a + 1);
}

void main1() {
  a = 42;
  uses_a();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  main1();
  return;
}
#version 310 es

int b = 0;
void uses_b() {
  b = (b * 2);
}

void main2() {
  b = 7;
  uses_b();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  main2();
  return;
}
#version 310 es

int a = 0;
int b = 0;
void uses_a() {
  a = (a + 1);
}

void uses_b() {
  b = (b * 2);
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

void main3() {
  outer();
  no_uses();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  main3();
  return;
}
#version 310 es

void no_uses() {
}

void main4() {
  no_uses();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  main4();
  return;
}

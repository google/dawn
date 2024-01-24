#version 310 es

struct a_block {
  int inner;
};

layout(location=0) uniform a_block a;
void uses_a() {
  int foo = a.inner;
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

struct a_block {
  int inner;
};

layout(location=0) uniform a_block a;
void uses_a() {
  int foo = a.inner;
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

struct b_block {
  int inner;
};

layout(location=0) uniform b_block b;
void uses_b() {
  int foo = b.inner;
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

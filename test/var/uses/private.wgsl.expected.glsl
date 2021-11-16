#version 310 es
precision mediump float;

int a = 0;

void uses_a() {
  a = (a + 1);
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main1() {
  a = 42;
  uses_a();
  return;
}
void main() {
  main1();
}


#version 310 es
precision mediump float;

int b = 0;

void uses_b() {
  b = (b * 2);
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main2() {
  b = 7;
  uses_b();
  return;
}
void main() {
  main2();
}


#version 310 es
precision mediump float;

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

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main3() {
  outer();
  no_uses();
  return;
}
void main() {
  main3();
}


#version 310 es
precision mediump float;

void no_uses() {
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main4() {
  no_uses();
  return;
}
void main() {
  main4();
}



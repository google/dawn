#version 310 es
precision highp float;

struct S {
  mat3 field0;
};

struct S_1 {
  S field0[47][21][83];
};

struct S_2 {
  vec3 field0[95][37];
};

struct S_3 {
  S_2 field0;
};

struct S_4 {
  ivec2 field0[56];
};

struct S_5 {
  S_4 field0;
};

struct S_6 {
  vec3 field0[13][18];
};

struct S_7 {
  ivec2 field0[88];
};

void main_1() {
  uint x_88 = 58u;
  return;
}

void tint_symbol() {
  main_1();
}

void main() {
  tint_symbol();
  return;
}

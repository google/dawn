#version 310 es

struct S {
  int before;
  uint pad;
  uint pad_1;
  uint pad_2;
  mat2x4 m;
  uint pad_3;
  uint pad_4;
  uint pad_5;
  uint pad_6;
  int after;
  uint pad_7;
  uint pad_8;
  uint pad_9;
  uint pad_10;
  uint pad_11;
  uint pad_12;
  uint pad_13;
  uint pad_14;
  uint pad_15;
  uint pad_16;
  uint pad_17;
  uint pad_18;
  uint pad_19;
  uint pad_20;
  uint pad_21;
};

layout(binding = 0, std140) uniform u_block_ubo {
  S inner[4];
} u;

void a(S a_1[4]) {
}

void b(S s) {
}

void c(mat2x4 m) {
}

void d(vec4 v) {
}

void e(float f_1) {
}

void f() {
  a(u.inner);
  b(u.inner[2]);
  c(u.inner[2].m);
  d(u.inner[0].m[1].ywxz);
  e(u.inner[0].m[1].ywxz.x);
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  f();
  return;
}

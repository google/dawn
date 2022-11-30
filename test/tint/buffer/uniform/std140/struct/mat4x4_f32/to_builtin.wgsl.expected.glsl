#version 310 es

struct S {
  int before;
  uint pad;
  uint pad_1;
  uint pad_2;
  mat4 m;
  uint pad_3;
  uint pad_4;
  uint pad_5;
  uint pad_6;
  uint pad_7;
  uint pad_8;
  uint pad_9;
  uint pad_10;
  uint pad_11;
  uint pad_12;
  uint pad_13;
  uint pad_14;
  int after;
  uint pad_15;
  uint pad_16;
  uint pad_17;
  uint pad_18;
  uint pad_19;
  uint pad_20;
  uint pad_21;
  uint pad_22;
  uint pad_23;
  uint pad_24;
  uint pad_25;
  uint pad_26;
  uint pad_27;
  uint pad_28;
  uint pad_29;
};

layout(binding = 0, std140) uniform u_block_ubo {
  S inner[4];
} u;

void f() {
  mat4 t = transpose(u.inner[2].m);
  float l = length(u.inner[0].m[1].ywxz);
  float a = abs(u.inner[0].m[1].ywxz.x);
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  f();
  return;
}

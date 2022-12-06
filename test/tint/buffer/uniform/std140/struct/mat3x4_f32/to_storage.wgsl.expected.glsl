#version 310 es

struct S {
  int before;
  uint pad;
  uint pad_1;
  uint pad_2;
  mat3x4 m;
  int after;
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
  uint pad_15;
  uint pad_16;
  uint pad_17;
};

layout(binding = 0, std140) uniform u_block_ubo {
  S inner[4];
} u;

layout(binding = 1, std430) buffer u_block_ssbo {
  S inner[4];
} s;

void assign_and_preserve_padding_1_s_X(uint dest[1], S value) {
  s.inner[dest[0]].before = value.before;
  s.inner[dest[0]].m = value.m;
  s.inner[dest[0]].after = value.after;
}

void assign_and_preserve_padding_s(S value[4]) {
  {
    for(uint i = 0u; (i < 4u); i = (i + 1u)) {
      uint tint_symbol[1] = uint[1](i);
      assign_and_preserve_padding_1_s_X(tint_symbol, value[i]);
    }
  }
}

void f() {
  assign_and_preserve_padding_s(u.inner);
  uint tint_symbol_1[1] = uint[1](1u);
  assign_and_preserve_padding_1_s_X(tint_symbol_1, u.inner[2]);
  s.inner[3].m = u.inner[2].m;
  s.inner[1].m[0] = u.inner[0].m[1].ywxz;
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  f();
  return;
}

#version 310 es

struct S {
  int before;
  uint pad;
  uint pad_1;
  uint pad_2;
  mat3 m;
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

void assign_and_preserve_padding_2_s_inner_X_m(uint dest[1], mat3 value) {
  s.inner[dest[0]].m[0] = value[0u];
  s.inner[dest[0]].m[1] = value[1u];
  s.inner[dest[0]].m[2] = value[2u];
}

void assign_and_preserve_padding_1_s_inner_X(uint dest[1], S value) {
  s.inner[dest[0]].before = value.before;
  uint tint_symbol[1] = uint[1](dest[0u]);
  assign_and_preserve_padding_2_s_inner_X_m(tint_symbol, value.m);
  s.inner[dest[0]].after = value.after;
}

void assign_and_preserve_padding_s_inner(S value[4]) {
  {
    for(uint i = 0u; (i < 4u); i = (i + 1u)) {
      uint tint_symbol_1[1] = uint[1](i);
      assign_and_preserve_padding_1_s_inner_X(tint_symbol_1, value[i]);
    }
  }
}

void f() {
  assign_and_preserve_padding_s_inner(u.inner);
  uint tint_symbol_2[1] = uint[1](1u);
  assign_and_preserve_padding_1_s_inner_X(tint_symbol_2, u.inner[2]);
  uint tint_symbol_3[1] = uint[1](3u);
  assign_and_preserve_padding_2_s_inner_X_m(tint_symbol_3, u.inner[2].m);
  s.inner[1].m[0] = u.inner[0].m[1].zxy;
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  f();
  return;
}

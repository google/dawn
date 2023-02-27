#version 310 es

layout(binding = 0, std140) uniform u_block_ubo {
  mat3 inner[4];
} u;

layout(binding = 1, std430) buffer u_block_ssbo {
  mat3 inner[4];
} s;

void assign_and_preserve_padding_1_s_X(uint dest[1], mat3 value) {
  s.inner[dest[0]][0] = value[0u];
  s.inner[dest[0]][1] = value[1u];
  s.inner[dest[0]][2] = value[2u];
}

void assign_and_preserve_padding_s(mat3 value[4]) {
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
  s.inner[1][0] = u.inner[0][1].zxy;
  s.inner[1][0].x = u.inner[0][1].x;
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  f();
  return;
}

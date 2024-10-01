#version 310 es


struct Buf {
  uint count;
  uint data[50];
};

layout(binding = 0, std430)
buffer tint_symbol_2_1_ssbo {
  Buf tint_symbol_1;
} v;
uint tint_mod_u32(uint lhs, uint rhs) {
  uint v_1 = mix(rhs, 1u, (rhs == 0u));
  return (lhs - ((lhs / v_1) * v_1));
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  uint i = 0u;
  {
    while(true) {
      if ((i >= v.tint_symbol_1.count)) {
        break;
      }
      uint v_2 = i;
      if ((tint_mod_u32(i, 2u) == 0u)) {
        {
          v.tint_symbol_1.data[v_2] = (v.tint_symbol_1.data[v_2] * 2u);
          i = (i + 1u);
        }
        continue;
      }
      v.tint_symbol_1.data[v_2] = 0u;
      {
        v.tint_symbol_1.data[v_2] = (v.tint_symbol_1.data[v_2] * 2u);
        i = (i + 1u);
      }
      continue;
    }
  }
}

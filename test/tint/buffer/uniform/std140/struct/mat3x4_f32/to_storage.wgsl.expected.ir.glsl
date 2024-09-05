#version 310 es


struct S {
  int before;
  mat3x4 m;
  int after;
};

layout(binding = 0, std140)
uniform tint_symbol_1_1_ubo {
  S tint_symbol[4];
} v;
layout(binding = 1, std430)
buffer tint_symbol_3_1_ssbo {
  S tint_symbol_2[4];
} v_1;
void tint_store_and_preserve_padding_1(inout S target, S value_param) {
  target.before = value_param.before;
  target.m = value_param.m;
  target.after = value_param.after;
}
void tint_store_and_preserve_padding(inout S target[4], S value_param[4]) {
  {
    uint v_2 = 0u;
    v_2 = 0u;
    while(true) {
      uint v_3 = v_2;
      if ((v_3 >= 4u)) {
        break;
      }
      tint_store_and_preserve_padding_1(target[v_3], value_param[v_3]);
      {
        v_2 = (v_3 + 1u);
      }
      continue;
    }
  }
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  tint_store_and_preserve_padding(v_1.tint_symbol_2, v.tint_symbol);
  tint_store_and_preserve_padding_1(v_1.tint_symbol_2[1], v.tint_symbol[2]);
  v_1.tint_symbol_2[3].m = v.tint_symbol[2].m;
  v_1.tint_symbol_2[1].m[0] = v.tint_symbol[0].m[1].ywxz;
}

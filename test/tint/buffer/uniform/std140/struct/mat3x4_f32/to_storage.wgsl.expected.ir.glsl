#version 310 es


struct S {
  int before;
  uint tint_pad_0;
  uint tint_pad_1;
  uint tint_pad_2;
  mat3x4 m;
  int after;
  uint tint_pad_3;
  uint tint_pad_4;
  uint tint_pad_5;
  uint tint_pad_6;
  uint tint_pad_7;
  uint tint_pad_8;
  uint tint_pad_9;
  uint tint_pad_10;
  uint tint_pad_11;
  uint tint_pad_12;
  uint tint_pad_13;
  uint tint_pad_14;
  uint tint_pad_15;
  uint tint_pad_16;
  uint tint_pad_17;
};

layout(binding = 0, std140)
uniform u_block_1_ubo {
  S inner[4];
} v;
layout(binding = 1, std430)
buffer s_block_1_ssbo {
  S inner[4];
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
  tint_store_and_preserve_padding(v_1.inner, v.inner);
  tint_store_and_preserve_padding_1(v_1.inner[1], v.inner[2]);
  v_1.inner[3].m = v.inner[2].m;
  v_1.inner[1].m[0] = v.inner[0].m[1].ywxz;
}

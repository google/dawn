#version 310 es


struct S {
  mat3 a[3];
  vec3 b[3][3];
};

layout(binding = 0, std430)
buffer s_block_1_ssbo {
  S inner;
} v;
void tint_store_and_preserve_padding_4(uint target_indices[1], vec3 value_param[3]) {
  {
    uint v_1 = 0u;
    v_1 = 0u;
    while(true) {
      uint v_2 = v_1;
      if ((v_2 >= 3u)) {
        break;
      }
      v.inner.b[target_indices[0u]][v_2] = value_param[v_2];
      {
        v_1 = (v_2 + 1u);
      }
    }
  }
}
void tint_store_and_preserve_padding_3(vec3 value_param[3][3]) {
  {
    uint v_3 = 0u;
    v_3 = 0u;
    while(true) {
      uint v_4 = v_3;
      if ((v_4 >= 3u)) {
        break;
      }
      tint_store_and_preserve_padding_4(uint[1](v_4), value_param[v_4]);
      {
        v_3 = (v_4 + 1u);
      }
    }
  }
}
void tint_store_and_preserve_padding_2(uint target_indices[1], mat3 value_param) {
  v.inner.a[target_indices[0u]][0u] = value_param[0u];
  v.inner.a[target_indices[0u]][1u] = value_param[1u];
  v.inner.a[target_indices[0u]][2u] = value_param[2u];
}
void tint_store_and_preserve_padding_1(mat3 value_param[3]) {
  {
    uint v_5 = 0u;
    v_5 = 0u;
    while(true) {
      uint v_6 = v_5;
      if ((v_6 >= 3u)) {
        break;
      }
      tint_store_and_preserve_padding_2(uint[1](v_6), value_param[v_6]);
      {
        v_5 = (v_6 + 1u);
      }
    }
  }
}
void tint_store_and_preserve_padding(S value_param) {
  tint_store_and_preserve_padding_1(value_param.a);
  tint_store_and_preserve_padding_3(value_param.b);
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  tint_store_and_preserve_padding(v.inner);
}

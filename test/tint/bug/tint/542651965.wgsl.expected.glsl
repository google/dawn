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
    uint idx = 0u;
    while(true) {
      uint v_1 = idx;
      if ((v_1 >= 3u)) {
        break;
      }
      v.inner.b[target_indices[0u]][v_1] = value_param[v_1];
      {
        idx = (idx + 1u);
      }
    }
  }
}
void tint_store_and_preserve_padding_3(vec3 value_param[3][3]) {
  {
    uint idx = 0u;
    while(true) {
      uint v_2 = idx;
      if ((v_2 >= 3u)) {
        break;
      }
      tint_store_and_preserve_padding_4(uint[1](v_2), value_param[v_2]);
      {
        idx = (idx + 1u);
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
    uint idx = 0u;
    while(true) {
      uint v_3 = idx;
      if ((v_3 >= 3u)) {
        break;
      }
      tint_store_and_preserve_padding_2(uint[1](v_3), value_param[v_3]);
      {
        idx = (idx + 1u);
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

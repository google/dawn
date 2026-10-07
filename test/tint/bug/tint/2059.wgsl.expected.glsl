#version 310 es


struct S {
  mat3 m;
};

struct S2 {
  mat3 m[1];
};

struct S3 {
  S s;
};

struct S4 {
  S s[1];
};

layout(binding = 0, std430)
buffer buffer0_block_1_ssbo {
  mat3 inner;
} v;
layout(binding = 1, std430)
buffer buffer1_block_1_ssbo {
  S inner;
} v_1;
layout(binding = 2, std430)
buffer buffer2_block_1_ssbo {
  S2 inner;
} v_2;
layout(binding = 3, std430)
buffer buffer3_block_1_ssbo {
  S3 inner;
} v_3;
layout(binding = 4, std430)
buffer buffer4_block_1_ssbo {
  S4 inner;
} v_4;
layout(binding = 5, std430)
buffer buffer5_block_1_ssbo {
  mat3 inner[1];
} v_5;
layout(binding = 6, std430)
buffer buffer6_block_1_ssbo {
  S inner[1];
} v_6;
layout(binding = 7, std430)
buffer buffer7_block_1_ssbo {
  S2 inner[1];
} v_7;
void tint_store_and_preserve_padding_1(uint target_indices[2], mat3 value_param) {
  v_7.inner[target_indices[0u]].m[target_indices[1u]][0u] = value_param[0u];
  v_7.inner[target_indices[0u]].m[target_indices[1u]][1u] = value_param[1u];
  v_7.inner[target_indices[0u]].m[target_indices[1u]][2u] = value_param[2u];
}
void tint_store_and_preserve_padding_15(uint target_indices[1], mat3 value_param[1]) {
  {
    uint idx = 0u;
    while(true) {
      uint v_8 = idx;
      if ((v_8 >= 1u)) {
        break;
      }
      tint_store_and_preserve_padding_1(uint[2](target_indices[0u], v_8), value_param[v_8]);
      {
        idx = (idx + 1u);
      }
    }
  }
}
void tint_store_and_preserve_padding_13(uint target_indices[1], S2 value_param) {
  tint_store_and_preserve_padding_15(uint[1](target_indices[0u]), value_param.m);
}
void tint_store_and_preserve_padding_21(S2 value_param[1]) {
  {
    uint idx = 0u;
    while(true) {
      uint v_9 = idx;
      if ((v_9 >= 1u)) {
        break;
      }
      tint_store_and_preserve_padding_13(uint[1](v_9), value_param[v_9]);
      {
        idx = (idx + 1u);
      }
    }
  }
}
void tint_store_and_preserve_padding_2(uint target_indices[1], mat3 value_param) {
  v_6.inner[target_indices[0u]].m[0u] = value_param[0u];
  v_6.inner[target_indices[0u]].m[1u] = value_param[1u];
  v_6.inner[target_indices[0u]].m[2u] = value_param[2u];
}
void tint_store_and_preserve_padding_9(uint target_indices[1], S value_param) {
  tint_store_and_preserve_padding_2(uint[1](target_indices[0u]), value_param.m);
}
void tint_store_and_preserve_padding_19(S value_param[1]) {
  {
    uint idx = 0u;
    while(true) {
      uint v_10 = idx;
      if ((v_10 >= 1u)) {
        break;
      }
      tint_store_and_preserve_padding_9(uint[1](v_10), value_param[v_10]);
      {
        idx = (idx + 1u);
      }
    }
  }
}
void tint_store_and_preserve_padding_3(uint target_indices[1], mat3 value_param) {
  v_5.inner[target_indices[0u]][0u] = value_param[0u];
  v_5.inner[target_indices[0u]][1u] = value_param[1u];
  v_5.inner[target_indices[0u]][2u] = value_param[2u];
}
void tint_store_and_preserve_padding_14(mat3 value_param[1]) {
  {
    uint idx = 0u;
    while(true) {
      uint v_11 = idx;
      if ((v_11 >= 1u)) {
        break;
      }
      tint_store_and_preserve_padding_3(uint[1](v_11), value_param[v_11]);
      {
        idx = (idx + 1u);
      }
    }
  }
}
void tint_store_and_preserve_padding_4(uint target_indices[1], mat3 value_param) {
  v_4.inner.s[target_indices[0u]].m[0u] = value_param[0u];
  v_4.inner.s[target_indices[0u]].m[1u] = value_param[1u];
  v_4.inner.s[target_indices[0u]].m[2u] = value_param[2u];
}
void tint_store_and_preserve_padding_10(uint target_indices[1], S value_param) {
  tint_store_and_preserve_padding_4(uint[1](target_indices[0u]), value_param.m);
}
void tint_store_and_preserve_padding_20(S value_param[1]) {
  {
    uint idx = 0u;
    while(true) {
      uint v_12 = idx;
      if ((v_12 >= 1u)) {
        break;
      }
      tint_store_and_preserve_padding_10(uint[1](v_12), value_param[v_12]);
      {
        idx = (idx + 1u);
      }
    }
  }
}
void tint_store_and_preserve_padding_18(S4 value_param) {
  tint_store_and_preserve_padding_20(value_param.s);
}
void tint_store_and_preserve_padding_5(mat3 value_param) {
  v_3.inner.s.m[0u] = value_param[0u];
  v_3.inner.s.m[1u] = value_param[1u];
  v_3.inner.s.m[2u] = value_param[2u];
}
void tint_store_and_preserve_padding_11(S value_param) {
  tint_store_and_preserve_padding_5(value_param.m);
}
void tint_store_and_preserve_padding_17(S3 value_param) {
  tint_store_and_preserve_padding_11(value_param.s);
}
void tint_store_and_preserve_padding_6(uint target_indices[1], mat3 value_param) {
  v_2.inner.m[target_indices[0u]][0u] = value_param[0u];
  v_2.inner.m[target_indices[0u]][1u] = value_param[1u];
  v_2.inner.m[target_indices[0u]][2u] = value_param[2u];
}
void tint_store_and_preserve_padding_16(mat3 value_param[1]) {
  {
    uint idx = 0u;
    while(true) {
      uint v_13 = idx;
      if ((v_13 >= 1u)) {
        break;
      }
      tint_store_and_preserve_padding_6(uint[1](v_13), value_param[v_13]);
      {
        idx = (idx + 1u);
      }
    }
  }
}
void tint_store_and_preserve_padding_12(S2 value_param) {
  tint_store_and_preserve_padding_16(value_param.m);
}
void tint_store_and_preserve_padding_7(mat3 value_param) {
  v_1.inner.m[0u] = value_param[0u];
  v_1.inner.m[1u] = value_param[1u];
  v_1.inner.m[2u] = value_param[2u];
}
void tint_store_and_preserve_padding_8(S value_param) {
  tint_store_and_preserve_padding_7(value_param.m);
}
void tint_store_and_preserve_padding(mat3 value_param) {
  v.inner[0u] = value_param[0u];
  v.inner[1u] = value_param[1u];
  v.inner[2u] = value_param[2u];
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  mat3 m = mat3(vec3(0.0f), vec3(0.0f), vec3(0.0f));
  {
    uint c = 0u;
    while(true) {
      if ((c < 3u)) {
      } else {
        break;
      }
      uint v_14 = c;
      float v_15 = float(((c * 3u) + 1u));
      float v_16 = float(((c * 3u) + 2u));
      m[v_14] = vec3(v_15, v_16, float(((c * 3u) + 3u)));
      {
        c = (c + 1u);
      }
    }
  }
  mat3 a = m;
  tint_store_and_preserve_padding(a);
  S a_1 = S(m);
  tint_store_and_preserve_padding_8(a_1);
  S2 a_2 = S2(mat3[1](m));
  tint_store_and_preserve_padding_12(a_2);
  S3 a_3 = S3(S(m));
  tint_store_and_preserve_padding_17(a_3);
  S4 a_4 = S4(S[1](S(m)));
  tint_store_and_preserve_padding_18(a_4);
  mat3 a_5[1] = mat3[1](m);
  tint_store_and_preserve_padding_14(a_5);
  S a_6[1] = S[1](S(m));
  tint_store_and_preserve_padding_19(a_6);
  S2 a_7[1] = S2[1](S2(mat3[1](m)));
  tint_store_and_preserve_padding_21(a_7);
}

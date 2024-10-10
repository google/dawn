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
void tint_store_and_preserve_padding(inout mat3 target, mat3 value_param) {
  target[0u] = value_param[0u];
  target[1u] = value_param[1u];
  target[2u] = value_param[2u];
}
void tint_store_and_preserve_padding_3(inout mat3 target[1], mat3 value_param[1]) {
  {
    uint v_8 = 0u;
    v_8 = 0u;
    while(true) {
      uint v_9 = v_8;
      if ((v_9 >= 1u)) {
        break;
      }
      tint_store_and_preserve_padding(target[v_9], value_param[v_9]);
      {
        v_8 = (v_9 + 1u);
      }
      continue;
    }
  }
}
void tint_store_and_preserve_padding_2(inout S2 target, S2 value_param) {
  tint_store_and_preserve_padding_3(target.m, value_param.m);
}
void tint_store_and_preserve_padding_7(inout S2 target[1], S2 value_param[1]) {
  {
    uint v_10 = 0u;
    v_10 = 0u;
    while(true) {
      uint v_11 = v_10;
      if ((v_11 >= 1u)) {
        break;
      }
      tint_store_and_preserve_padding_2(target[v_11], value_param[v_11]);
      {
        v_10 = (v_11 + 1u);
      }
      continue;
    }
  }
}
void tint_store_and_preserve_padding_1(inout S target, S value_param) {
  tint_store_and_preserve_padding(target.m, value_param.m);
}
void tint_store_and_preserve_padding_6(inout S target[1], S value_param[1]) {
  {
    uint v_12 = 0u;
    v_12 = 0u;
    while(true) {
      uint v_13 = v_12;
      if ((v_13 >= 1u)) {
        break;
      }
      tint_store_and_preserve_padding_1(target[v_13], value_param[v_13]);
      {
        v_12 = (v_13 + 1u);
      }
      continue;
    }
  }
}
void tint_store_and_preserve_padding_5(inout S4 target, S4 value_param) {
  tint_store_and_preserve_padding_6(target.s, value_param.s);
}
void tint_store_and_preserve_padding_4(inout S3 target, S3 value_param) {
  tint_store_and_preserve_padding_1(target.s, value_param.s);
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
      continue;
    }
  }
  mat3 a = m;
  tint_store_and_preserve_padding(v.inner, a);
  S a_1 = S(m);
  tint_store_and_preserve_padding_1(v_1.inner, a_1);
  S2 a_2 = S2(mat3[1](m));
  tint_store_and_preserve_padding_2(v_2.inner, a_2);
  S3 a_3 = S3(S(m));
  tint_store_and_preserve_padding_4(v_3.inner, a_3);
  S4 a_4 = S4(S[1](S(m)));
  tint_store_and_preserve_padding_5(v_4.inner, a_4);
  mat3 a_5[1] = mat3[1](m);
  tint_store_and_preserve_padding_3(v_5.inner, a_5);
  S a_6[1] = S[1](S(m));
  tint_store_and_preserve_padding_6(v_6.inner, a_6);
  S2 a_7[1] = S2[1](S2(mat3[1](m)));
  tint_store_and_preserve_padding_7(v_7.inner, a_7);
}

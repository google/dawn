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

layout(binding = 0, std430) buffer buffer0_block_ssbo {
  mat3 inner;
} buffer0;

layout(binding = 1, std430) buffer buffer1_block_ssbo {
  S inner;
} buffer1;

layout(binding = 2, std430) buffer buffer2_block_ssbo {
  S2 inner;
} buffer2;

layout(binding = 3, std430) buffer buffer3_block_ssbo {
  S3 inner;
} buffer3;

layout(binding = 4, std430) buffer buffer4_block_ssbo {
  S4 inner;
} buffer4;

layout(binding = 5, std430) buffer buffer5_block_ssbo {
  mat3 inner[1];
} buffer5;

layout(binding = 6, std430) buffer buffer6_block_ssbo {
  S inner[1];
} buffer6;

layout(binding = 7, std430) buffer buffer7_block_ssbo {
  S2 inner[1];
} buffer7;

void assign_and_preserve_padding_buffer0(mat3 value) {
  buffer0.inner[0] = value[0u];
  buffer0.inner[1] = value[1u];
  buffer0.inner[2] = value[2u];
}

void assign_and_preserve_padding_buffer5_X(uint dest[1], mat3 value) {
  buffer5.inner[dest[0]][0] = value[0u];
  buffer5.inner[dest[0]][1] = value[1u];
  buffer5.inner[dest[0]][2] = value[2u];
}

void assign_and_preserve_padding_buffer2_m_X(uint dest[1], mat3 value) {
  buffer2.inner.m[dest[0]][0] = value[0u];
  buffer2.inner.m[dest[0]][1] = value[1u];
  buffer2.inner.m[dest[0]][2] = value[2u];
}

void assign_and_preserve_padding_buffer7_X_m_X(uint dest[2], mat3 value) {
  buffer7.inner[dest[0]].m[dest[0]][0] = value[0u];
  buffer7.inner[dest[0]].m[dest[0]][1] = value[1u];
  buffer7.inner[dest[0]].m[dest[0]][2] = value[2u];
}

void assign_and_preserve_padding_buffer1_m(mat3 value) {
  buffer1.inner.m[0] = value[0u];
  buffer1.inner.m[1] = value[1u];
  buffer1.inner.m[2] = value[2u];
}

void assign_and_preserve_padding_buffer6_X_m(uint dest[1], mat3 value) {
  buffer6.inner[dest[0]].m[0] = value[0u];
  buffer6.inner[dest[0]].m[1] = value[1u];
  buffer6.inner[dest[0]].m[2] = value[2u];
}

void assign_and_preserve_padding_buffer4_s_X_m(uint dest[1], mat3 value) {
  buffer4.inner.s[dest[0]].m[0] = value[0u];
  buffer4.inner.s[dest[0]].m[1] = value[1u];
  buffer4.inner.s[dest[0]].m[2] = value[2u];
}

void assign_and_preserve_padding_buffer3_s_m(mat3 value) {
  buffer3.inner.s.m[0] = value[0u];
  buffer3.inner.s.m[1] = value[1u];
  buffer3.inner.s.m[2] = value[2u];
}

void assign_and_preserve_padding_1_buffer1(S value) {
  assign_and_preserve_padding_buffer1_m(value.m);
}

void assign_and_preserve_padding_1_buffer6_X(uint dest[1], S value) {
  uint tint_symbol_1[1] = uint[1](dest[0u]);
  assign_and_preserve_padding_buffer6_X_m(tint_symbol_1, value.m);
}

void assign_and_preserve_padding_1_buffer4_s_X(uint dest[1], S value) {
  uint tint_symbol_2[1] = uint[1](dest[0u]);
  assign_and_preserve_padding_buffer4_s_X_m(tint_symbol_2, value.m);
}

void assign_and_preserve_padding_1_buffer3_s(S value) {
  assign_and_preserve_padding_buffer3_s_m(value.m);
}

void assign_and_preserve_padding_3_buffer5(mat3 value[1]) {
  {
    for(uint i = 0u; (i < 1u); i = (i + 1u)) {
      uint tint_symbol_3[1] = uint[1](i);
      assign_and_preserve_padding_buffer5_X(tint_symbol_3, value[i]);
    }
  }
}

void assign_and_preserve_padding_3_buffer2_m(mat3 value[1]) {
  {
    for(uint i = 0u; (i < 1u); i = (i + 1u)) {
      uint tint_symbol_4[1] = uint[1](i);
      assign_and_preserve_padding_buffer2_m_X(tint_symbol_4, value[i]);
    }
  }
}

void assign_and_preserve_padding_3_buffer7_X_m(uint dest[1], mat3 value[1]) {
  {
    for(uint i = 0u; (i < 1u); i = (i + 1u)) {
      uint tint_symbol_5[2] = uint[2](dest[0u], i);
      assign_and_preserve_padding_buffer7_X_m_X(tint_symbol_5, value[i]);
    }
  }
}

void assign_and_preserve_padding_2_buffer2(S2 value) {
  assign_and_preserve_padding_3_buffer2_m(value.m);
}

void assign_and_preserve_padding_2_buffer7_X(uint dest[1], S2 value) {
  uint tint_symbol_6[1] = uint[1](dest[0u]);
  assign_and_preserve_padding_3_buffer7_X_m(tint_symbol_6, value.m);
}

void assign_and_preserve_padding_4_buffer3(S3 value) {
  assign_and_preserve_padding_1_buffer3_s(value.s);
}

void assign_and_preserve_padding_6_buffer6(S value[1]) {
  {
    for(uint i = 0u; (i < 1u); i = (i + 1u)) {
      uint tint_symbol_7[1] = uint[1](i);
      assign_and_preserve_padding_1_buffer6_X(tint_symbol_7, value[i]);
    }
  }
}

void assign_and_preserve_padding_6_buffer4_s(S value[1]) {
  {
    for(uint i = 0u; (i < 1u); i = (i + 1u)) {
      uint tint_symbol_8[1] = uint[1](i);
      assign_and_preserve_padding_1_buffer4_s_X(tint_symbol_8, value[i]);
    }
  }
}

void assign_and_preserve_padding_5_buffer4(S4 value) {
  assign_and_preserve_padding_6_buffer4_s(value.s);
}

void assign_and_preserve_padding_7_buffer7(S2 value[1]) {
  {
    for(uint i = 0u; (i < 1u); i = (i + 1u)) {
      uint tint_symbol_9[1] = uint[1](i);
      assign_and_preserve_padding_2_buffer7_X(tint_symbol_9, value[i]);
    }
  }
}

void tint_symbol() {
  mat3 m = mat3(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
  {
    for(uint c = 0u; (c < 3u); c = (c + 1u)) {
      m[c] = vec3(float(((c * 3u) + 1u)), float(((c * 3u) + 2u)), float(((c * 3u) + 3u)));
    }
  }
  {
    mat3 a = m;
    assign_and_preserve_padding_buffer0(a);
  }
  {
    S a = S(m);
    assign_and_preserve_padding_1_buffer1(a);
  }
  {
    mat3 tint_symbol_10[1] = mat3[1](m);
    S2 a = S2(tint_symbol_10);
    assign_and_preserve_padding_2_buffer2(a);
  }
  {
    S tint_symbol_11 = S(m);
    S3 a = S3(tint_symbol_11);
    assign_and_preserve_padding_4_buffer3(a);
  }
  {
    S tint_symbol_12 = S(m);
    S tint_symbol_13[1] = S[1](tint_symbol_12);
    S4 a = S4(tint_symbol_13);
    assign_and_preserve_padding_5_buffer4(a);
  }
  {
    mat3 a[1] = mat3[1](m);
    assign_and_preserve_padding_3_buffer5(a);
  }
  {
    S tint_symbol_14 = S(m);
    S a[1] = S[1](tint_symbol_14);
    assign_and_preserve_padding_6_buffer6(a);
  }
  {
    mat3 tint_symbol_15[1] = mat3[1](m);
    S2 tint_symbol_16 = S2(tint_symbol_15);
    S2 a[1] = S2[1](tint_symbol_16);
    assign_and_preserve_padding_7_buffer7(a);
  }
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  tint_symbol();
  return;
}

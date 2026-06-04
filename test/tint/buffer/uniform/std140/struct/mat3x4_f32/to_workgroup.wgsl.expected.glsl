#version 310 es


struct S {
  int before;
  mat3x4 m;
  int after;
};

layout(binding = 0, std140)
uniform u_block_1_ubo {
  uvec4 inner[32];
} v;
shared S w[4];
mat3x4 v_1(uint start_byte_offset) {
  return mat3x4(uintBitsToFloat(v.inner[(start_byte_offset / 16u)]), uintBitsToFloat(v.inner[((16u + start_byte_offset) / 16u)]), uintBitsToFloat(v.inner[((32u + start_byte_offset) / 16u)]));
}
S v_2(uint start_byte_offset) {
  uvec4 v_3 = v.inner[(start_byte_offset / 16u)];
  int v_4 = int(v_3[((start_byte_offset & 15u) >> 2u)]);
  mat3x4 v_5 = v_1((16u + start_byte_offset));
  uint v_6 = (64u + start_byte_offset);
  uvec4 v_7 = v.inner[(v_6 / 16u)];
  return S(v_4, v_5, int(v_7[((v_6 & 15u) >> 2u)]));
}
S[4] v_8(uint start_byte_offset) {
  S a[4] = S[4](S(0, mat3x4(vec4(0.0f), vec4(0.0f), vec4(0.0f)), 0), S(0, mat3x4(vec4(0.0f), vec4(0.0f), vec4(0.0f)), 0), S(0, mat3x4(vec4(0.0f), vec4(0.0f), vec4(0.0f)), 0), S(0, mat3x4(vec4(0.0f), vec4(0.0f), vec4(0.0f)), 0));
  {
    uint v_9 = 0u;
    v_9 = 0u;
    while(true) {
      uint v_10 = v_9;
      if ((v_10 >= 4u)) {
        break;
      }
      a[v_10] = v_2((start_byte_offset + (v_10 * 128u)));
      {
        v_9 = (v_10 + 1u);
      }
    }
  }
  return a;
}
void f_inner(uint tint_local_index) {
  {
    uint v_11 = 0u;
    v_11 = tint_local_index;
    while(true) {
      uint v_12 = v_11;
      if ((v_12 >= 4u)) {
        break;
      }
      w[v_12] = S(0, mat3x4(vec4(0.0f), vec4(0.0f), vec4(0.0f)), 0);
      {
        v_11 = (v_12 + 1u);
      }
    }
  }
  barrier();
  w = v_8(0u);
  w[1u] = v_2(256u);
  w[3u].m = v_1(272u);
  w[1u].m[0u] = uintBitsToFloat(v.inner[2u]).ywxz;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  f_inner(gl_LocalInvocationIndex);
}

#version 310 es


struct S {
  int before;
  mat2x4 m;
  int after;
};

layout(binding = 0, std140)
uniform u_block_1_ubo {
  uvec4 inner[32];
} v;
shared S w[4];
mat2x4 v_1(uint start_byte_offset) {
  return mat2x4(uintBitsToFloat(v.inner[(start_byte_offset / 16u)]), uintBitsToFloat(v.inner[((16u + start_byte_offset) / 16u)]));
}
S v_2(uint start_byte_offset) {
  uvec4 v_3 = v.inner[(start_byte_offset / 16u)];
  int v_4 = int(v_3[((start_byte_offset % 16u) / 4u)]);
  mat2x4 v_5 = v_1((16u + start_byte_offset));
  uvec4 v_6 = v.inner[((64u + start_byte_offset) / 16u)];
  return S(v_4, v_5, int(v_6[(((64u + start_byte_offset) % 16u) / 4u)]));
}
S[4] v_7(uint start_byte_offset) {
  S a[4] = S[4](S(0, mat2x4(vec4(0.0f), vec4(0.0f)), 0), S(0, mat2x4(vec4(0.0f), vec4(0.0f)), 0), S(0, mat2x4(vec4(0.0f), vec4(0.0f)), 0), S(0, mat2x4(vec4(0.0f), vec4(0.0f)), 0));
  {
    uint v_8 = 0u;
    v_8 = 0u;
    while(true) {
      uint v_9 = v_8;
      if ((v_9 >= 4u)) {
        break;
      }
      a[v_9] = v_2((start_byte_offset + (v_9 * 128u)));
      {
        v_8 = (v_9 + 1u);
      }
      continue;
    }
  }
  return a;
}
void f_inner(uint tint_local_index) {
  {
    uint v_10 = 0u;
    v_10 = tint_local_index;
    while(true) {
      uint v_11 = v_10;
      if ((v_11 >= 4u)) {
        break;
      }
      w[v_11] = S(0, mat2x4(vec4(0.0f), vec4(0.0f)), 0);
      {
        v_10 = (v_11 + 1u);
      }
      continue;
    }
  }
  barrier();
  w = v_7(0u);
  w[1u] = v_2(256u);
  w[3u].m = v_1(272u);
  w[1u].m[0u] = uintBitsToFloat(v.inner[2u]).ywxz;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  f_inner(gl_LocalInvocationIndex);
}

#version 310 es


struct S {
  int before;
  mat3x2 m;
  int after;
};

layout(binding = 0, std140)
uniform u_block_1_ubo {
  uvec4 inner[32];
} v;
shared S w[4];
mat3x2 v_1(uint start_byte_offset) {
  uvec4 v_2 = v.inner[(start_byte_offset / 16u)];
  vec2 v_3 = uintBitsToFloat(mix(v_2.xy, v_2.zw, bvec2((((start_byte_offset & 15u) >> 2u) == 2u))));
  uvec4 v_4 = v.inner[((8u + start_byte_offset) / 16u)];
  vec2 v_5 = uintBitsToFloat(mix(v_4.xy, v_4.zw, bvec2(((((8u + start_byte_offset) & 15u) >> 2u) == 2u))));
  uvec4 v_6 = v.inner[((16u + start_byte_offset) / 16u)];
  return mat3x2(v_3, v_5, uintBitsToFloat(mix(v_6.xy, v_6.zw, bvec2(((((16u + start_byte_offset) & 15u) >> 2u) == 2u)))));
}
S v_7(uint start_byte_offset) {
  uvec4 v_8 = v.inner[(start_byte_offset / 16u)];
  int v_9 = int(v_8[((start_byte_offset & 15u) >> 2u)]);
  mat3x2 v_10 = v_1((8u + start_byte_offset));
  uvec4 v_11 = v.inner[((64u + start_byte_offset) / 16u)];
  return S(v_9, v_10, int(v_11[(((64u + start_byte_offset) & 15u) >> 2u)]));
}
S[4] v_12(uint start_byte_offset) {
  S a[4] = S[4](S(0, mat3x2(vec2(0.0f), vec2(0.0f), vec2(0.0f)), 0), S(0, mat3x2(vec2(0.0f), vec2(0.0f), vec2(0.0f)), 0), S(0, mat3x2(vec2(0.0f), vec2(0.0f), vec2(0.0f)), 0), S(0, mat3x2(vec2(0.0f), vec2(0.0f), vec2(0.0f)), 0));
  {
    uint v_13 = 0u;
    v_13 = 0u;
    while(true) {
      uint v_14 = v_13;
      if ((v_14 >= 4u)) {
        break;
      }
      a[v_14] = v_7((start_byte_offset + (v_14 * 128u)));
      {
        v_13 = (v_14 + 1u);
      }
      continue;
    }
  }
  return a;
}
void f_inner(uint tint_local_index) {
  {
    uint v_15 = 0u;
    v_15 = tint_local_index;
    while(true) {
      uint v_16 = v_15;
      if ((v_16 >= 4u)) {
        break;
      }
      w[v_16] = S(0, mat3x2(vec2(0.0f), vec2(0.0f), vec2(0.0f)), 0);
      {
        v_15 = (v_16 + 1u);
      }
      continue;
    }
  }
  barrier();
  w = v_12(0u);
  w[1u] = v_7(256u);
  w[3u].m = v_1(264u);
  w[1u].m[0u] = uintBitsToFloat(v.inner[1u].xy).yx;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  f_inner(gl_LocalInvocationIndex);
}

#version 310 es


struct S {
  int before;
  mat4x2 m;
  int after;
};

layout(binding = 0, std140)
uniform u_block_1_ubo {
  uvec4 inner[32];
} v;
shared S w[4];
mat4x2 v_1(uint start_byte_offset) {
  uvec4 v_2 = v.inner[(start_byte_offset / 16u)];
  vec2 v_3 = uintBitsToFloat(mix(v_2.xy, v_2.zw, bvec2((((start_byte_offset & 15u) >> 2u) == 2u))));
  uint v_4 = (8u + start_byte_offset);
  uvec4 v_5 = v.inner[(v_4 / 16u)];
  vec2 v_6 = uintBitsToFloat(mix(v_5.xy, v_5.zw, bvec2((((v_4 & 15u) >> 2u) == 2u))));
  uint v_7 = (16u + start_byte_offset);
  uvec4 v_8 = v.inner[(v_7 / 16u)];
  vec2 v_9 = uintBitsToFloat(mix(v_8.xy, v_8.zw, bvec2((((v_7 & 15u) >> 2u) == 2u))));
  uint v_10 = (24u + start_byte_offset);
  uvec4 v_11 = v.inner[(v_10 / 16u)];
  return mat4x2(v_3, v_6, v_9, uintBitsToFloat(mix(v_11.xy, v_11.zw, bvec2((((v_10 & 15u) >> 2u) == 2u)))));
}
S v_12(uint start_byte_offset) {
  uvec4 v_13 = v.inner[(start_byte_offset / 16u)];
  int v_14 = int(v_13[((start_byte_offset & 15u) >> 2u)]);
  mat4x2 v_15 = v_1((8u + start_byte_offset));
  uint v_16 = (64u + start_byte_offset);
  uvec4 v_17 = v.inner[(v_16 / 16u)];
  return S(v_14, v_15, int(v_17[((v_16 & 15u) >> 2u)]));
}
S[4] v_18(uint start_byte_offset) {
  S a[4] = S[4](S(0, mat4x2(vec2(0.0f), vec2(0.0f), vec2(0.0f), vec2(0.0f)), 0), S(0, mat4x2(vec2(0.0f), vec2(0.0f), vec2(0.0f), vec2(0.0f)), 0), S(0, mat4x2(vec2(0.0f), vec2(0.0f), vec2(0.0f), vec2(0.0f)), 0), S(0, mat4x2(vec2(0.0f), vec2(0.0f), vec2(0.0f), vec2(0.0f)), 0));
  {
    uint v_19 = 0u;
    v_19 = 0u;
    while(true) {
      uint v_20 = v_19;
      if ((v_20 >= 4u)) {
        break;
      }
      a[v_20] = v_12((start_byte_offset + (v_20 * 128u)));
      {
        v_19 = (v_20 + 1u);
      }
    }
  }
  return a;
}
void f_inner(uint tint_local_index) {
  {
    uint v_21 = 0u;
    v_21 = tint_local_index;
    while(true) {
      uint v_22 = v_21;
      if ((v_22 >= 4u)) {
        break;
      }
      w[v_22] = S(0, mat4x2(vec2(0.0f), vec2(0.0f), vec2(0.0f), vec2(0.0f)), 0);
      {
        v_21 = (v_22 + 1u);
      }
    }
  }
  barrier();
  w = v_18(0u);
  w[1u] = v_12(256u);
  w[3u].m = v_1(264u);
  w[1u].m[0u] = uintBitsToFloat(v.inner[1u].xy).yx;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  f_inner(gl_LocalInvocationIndex);
}

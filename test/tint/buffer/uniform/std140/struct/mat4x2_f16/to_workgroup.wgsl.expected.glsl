#version 310 es
#extension GL_AMD_gpu_shader_half_float: require


struct S {
  int before;
  f16mat4x2 m;
  int after;
};

layout(binding = 0, std140)
uniform u_block_1_ubo {
  uvec4 inner[32];
} v;
shared S w[4];
f16vec2 tint_bitcast_to_16bit(uint src) {
  return unpackFloat2x16(src);
}
f16mat4x2 v_1(uint start_byte_offset) {
  f16vec2 v_2 = tint_bitcast_to_16bit(v.inner[(start_byte_offset / 16u)][((start_byte_offset & 15u) >> 2u)]);
  uint v_3 = (4u + start_byte_offset);
  f16vec2 v_4 = tint_bitcast_to_16bit(v.inner[(v_3 / 16u)][((v_3 & 15u) >> 2u)]);
  uint v_5 = (8u + start_byte_offset);
  f16vec2 v_6 = tint_bitcast_to_16bit(v.inner[(v_5 / 16u)][((v_5 & 15u) >> 2u)]);
  uint v_7 = (12u + start_byte_offset);
  return f16mat4x2(v_2, v_4, v_6, tint_bitcast_to_16bit(v.inner[(v_7 / 16u)][((v_7 & 15u) >> 2u)]));
}
S v_8(uint start_byte_offset) {
  uvec4 v_9 = v.inner[(start_byte_offset / 16u)];
  int v_10 = int(v_9[((start_byte_offset & 15u) >> 2u)]);
  f16mat4x2 v_11 = v_1((4u + start_byte_offset));
  uint v_12 = (64u + start_byte_offset);
  uvec4 v_13 = v.inner[(v_12 / 16u)];
  return S(v_10, v_11, int(v_13[((v_12 & 15u) >> 2u)]));
}
S[4] v_14(uint start_byte_offset) {
  S a[4] = S[4](S(0, f16mat4x2(f16vec2(0.0hf), f16vec2(0.0hf), f16vec2(0.0hf), f16vec2(0.0hf)), 0), S(0, f16mat4x2(f16vec2(0.0hf), f16vec2(0.0hf), f16vec2(0.0hf), f16vec2(0.0hf)), 0), S(0, f16mat4x2(f16vec2(0.0hf), f16vec2(0.0hf), f16vec2(0.0hf), f16vec2(0.0hf)), 0), S(0, f16mat4x2(f16vec2(0.0hf), f16vec2(0.0hf), f16vec2(0.0hf), f16vec2(0.0hf)), 0));
  {
    uint v_15 = 0u;
    v_15 = 0u;
    while(true) {
      uint v_16 = v_15;
      if ((v_16 >= 4u)) {
        break;
      }
      a[v_16] = v_8((start_byte_offset + (v_16 * 128u)));
      {
        v_15 = (v_16 + 1u);
      }
    }
  }
  return a;
}
void f_inner(uint tint_local_index) {
  {
    uint v_17 = 0u;
    v_17 = tint_local_index;
    while(true) {
      uint v_18 = v_17;
      if ((v_18 >= 4u)) {
        break;
      }
      w[v_18] = S(0, f16mat4x2(f16vec2(0.0hf), f16vec2(0.0hf), f16vec2(0.0hf), f16vec2(0.0hf)), 0);
      {
        v_17 = (v_18 + 1u);
      }
    }
  }
  barrier();
  w = v_14(0u);
  w[1u] = v_8(256u);
  w[3u].m = v_1(260u);
  w[1u].m[0u] = tint_bitcast_to_16bit(v.inner[0u].z).yx;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  f_inner(gl_LocalInvocationIndex);
}

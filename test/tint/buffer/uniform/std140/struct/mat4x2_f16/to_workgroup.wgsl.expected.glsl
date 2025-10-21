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
f16vec2 tint_bitcast_to_f16(uint src) {
  return unpackFloat2x16(src);
}
f16mat4x2 v_1(uint start_byte_offset) {
  f16vec2 v_2 = tint_bitcast_to_f16(v.inner[(start_byte_offset / 16u)][((start_byte_offset % 16u) / 4u)]);
  f16vec2 v_3 = tint_bitcast_to_f16(v.inner[((4u + start_byte_offset) / 16u)][(((4u + start_byte_offset) % 16u) / 4u)]);
  f16vec2 v_4 = tint_bitcast_to_f16(v.inner[((8u + start_byte_offset) / 16u)][(((8u + start_byte_offset) % 16u) / 4u)]);
  return f16mat4x2(v_2, v_3, v_4, tint_bitcast_to_f16(v.inner[((12u + start_byte_offset) / 16u)][(((12u + start_byte_offset) % 16u) / 4u)]));
}
S v_5(uint start_byte_offset) {
  uvec4 v_6 = v.inner[(start_byte_offset / 16u)];
  int v_7 = int(v_6[((start_byte_offset % 16u) / 4u)]);
  f16mat4x2 v_8 = v_1((4u + start_byte_offset));
  uvec4 v_9 = v.inner[((64u + start_byte_offset) / 16u)];
  return S(v_7, v_8, int(v_9[(((64u + start_byte_offset) % 16u) / 4u)]));
}
S[4] v_10(uint start_byte_offset) {
  S a[4] = S[4](S(0, f16mat4x2(f16vec2(0.0hf), f16vec2(0.0hf), f16vec2(0.0hf), f16vec2(0.0hf)), 0), S(0, f16mat4x2(f16vec2(0.0hf), f16vec2(0.0hf), f16vec2(0.0hf), f16vec2(0.0hf)), 0), S(0, f16mat4x2(f16vec2(0.0hf), f16vec2(0.0hf), f16vec2(0.0hf), f16vec2(0.0hf)), 0), S(0, f16mat4x2(f16vec2(0.0hf), f16vec2(0.0hf), f16vec2(0.0hf), f16vec2(0.0hf)), 0));
  {
    uint v_11 = 0u;
    v_11 = 0u;
    while(true) {
      uint v_12 = v_11;
      if ((v_12 >= 4u)) {
        break;
      }
      a[v_12] = v_5((start_byte_offset + (v_12 * 128u)));
      {
        v_11 = (v_12 + 1u);
      }
      continue;
    }
  }
  return a;
}
void f_inner(uint tint_local_index) {
  {
    uint v_13 = 0u;
    v_13 = tint_local_index;
    while(true) {
      uint v_14 = v_13;
      if ((v_14 >= 4u)) {
        break;
      }
      w[v_14] = S(0, f16mat4x2(f16vec2(0.0hf), f16vec2(0.0hf), f16vec2(0.0hf), f16vec2(0.0hf)), 0);
      {
        v_13 = (v_14 + 1u);
      }
      continue;
    }
  }
  barrier();
  w = v_10(0u);
  w[1u] = v_5(256u);
  w[3u].m = v_1(260u);
  w[1u].m[0u] = tint_bitcast_to_f16(v.inner[0u].z).yx;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  f_inner(gl_LocalInvocationIndex);
}

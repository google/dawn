#version 310 es
#extension GL_AMD_gpu_shader_half_float: require


struct S {
  int before;
  f16mat2 m;
  int after;
};

layout(binding = 0, std140)
uniform u_block_1_ubo {
  uvec4 inner[32];
} v;
S p[4] = S[4](S(0, f16mat2(f16vec2(0.0hf), f16vec2(0.0hf)), 0), S(0, f16mat2(f16vec2(0.0hf), f16vec2(0.0hf)), 0), S(0, f16mat2(f16vec2(0.0hf), f16vec2(0.0hf)), 0), S(0, f16mat2(f16vec2(0.0hf), f16vec2(0.0hf)), 0));
f16vec2 tint_bitcast_to_f16(uint src) {
  return unpackFloat2x16(src);
}
f16mat2 v_1(uint start_byte_offset) {
  f16vec2 v_2 = tint_bitcast_to_f16(v.inner[(start_byte_offset / 16u)][((start_byte_offset & 15u) >> 2u)]);
  return f16mat2(v_2, tint_bitcast_to_f16(v.inner[((4u + start_byte_offset) / 16u)][(((4u + start_byte_offset) & 15u) >> 2u)]));
}
S v_3(uint start_byte_offset) {
  uvec4 v_4 = v.inner[(start_byte_offset / 16u)];
  int v_5 = int(v_4[((start_byte_offset & 15u) >> 2u)]);
  f16mat2 v_6 = v_1((4u + start_byte_offset));
  uvec4 v_7 = v.inner[((64u + start_byte_offset) / 16u)];
  return S(v_5, v_6, int(v_7[(((64u + start_byte_offset) & 15u) >> 2u)]));
}
S[4] v_8(uint start_byte_offset) {
  S a[4] = S[4](S(0, f16mat2(f16vec2(0.0hf), f16vec2(0.0hf)), 0), S(0, f16mat2(f16vec2(0.0hf), f16vec2(0.0hf)), 0), S(0, f16mat2(f16vec2(0.0hf), f16vec2(0.0hf)), 0), S(0, f16mat2(f16vec2(0.0hf), f16vec2(0.0hf)), 0));
  {
    uint v_9 = 0u;
    v_9 = 0u;
    while(true) {
      uint v_10 = v_9;
      if ((v_10 >= 4u)) {
        break;
      }
      a[v_10] = v_3((start_byte_offset + (v_10 * 128u)));
      {
        v_9 = (v_10 + 1u);
      }
      continue;
    }
  }
  return a;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  p = v_8(0u);
  p[1u] = v_3(256u);
  p[3u].m = v_1(260u);
  p[1u].m[0u] = tint_bitcast_to_f16(v.inner[0u].z).yx;
}

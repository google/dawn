#version 310 es


struct S {
  int before;
  mat4x3 m;
  int after;
};

layout(binding = 0, std140)
uniform u_block_1_ubo {
  uvec4 inner[48];
} v;
S p[4] = S[4](S(0, mat4x3(vec3(0.0f), vec3(0.0f), vec3(0.0f), vec3(0.0f)), 0), S(0, mat4x3(vec3(0.0f), vec3(0.0f), vec3(0.0f), vec3(0.0f)), 0), S(0, mat4x3(vec3(0.0f), vec3(0.0f), vec3(0.0f), vec3(0.0f)), 0), S(0, mat4x3(vec3(0.0f), vec3(0.0f), vec3(0.0f), vec3(0.0f)), 0));
mat4x3 v_1(uint start_byte_offset) {
  return mat4x3(uintBitsToFloat(v.inner[(start_byte_offset / 16u)].xyz), uintBitsToFloat(v.inner[((16u + start_byte_offset) / 16u)].xyz), uintBitsToFloat(v.inner[((32u + start_byte_offset) / 16u)].xyz), uintBitsToFloat(v.inner[((48u + start_byte_offset) / 16u)].xyz));
}
S v_2(uint start_byte_offset) {
  uvec4 v_3 = v.inner[(start_byte_offset / 16u)];
  int v_4 = int(v_3[((start_byte_offset & 15u) >> 2u)]);
  mat4x3 v_5 = v_1((16u + start_byte_offset));
  uvec4 v_6 = v.inner[((128u + start_byte_offset) / 16u)];
  return S(v_4, v_5, int(v_6[(((128u + start_byte_offset) & 15u) >> 2u)]));
}
S[4] v_7(uint start_byte_offset) {
  S a[4] = S[4](S(0, mat4x3(vec3(0.0f), vec3(0.0f), vec3(0.0f), vec3(0.0f)), 0), S(0, mat4x3(vec3(0.0f), vec3(0.0f), vec3(0.0f), vec3(0.0f)), 0), S(0, mat4x3(vec3(0.0f), vec3(0.0f), vec3(0.0f), vec3(0.0f)), 0), S(0, mat4x3(vec3(0.0f), vec3(0.0f), vec3(0.0f), vec3(0.0f)), 0));
  {
    uint v_8 = 0u;
    v_8 = 0u;
    while(true) {
      uint v_9 = v_8;
      if ((v_9 >= 4u)) {
        break;
      }
      a[v_9] = v_2((start_byte_offset + (v_9 * 192u)));
      {
        v_8 = (v_9 + 1u);
      }
      continue;
    }
  }
  return a;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  p = v_7(0u);
  p[1u] = v_2(384u);
  p[3u].m = v_1(400u);
  p[1u].m[0u] = uintBitsToFloat(v.inner[2u].xyz).zxy;
}

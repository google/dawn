#version 310 es

layout(binding = 0, std140)
uniform u_block_1_ubo {
  uvec4 inner[12];
} v;
shared mat3 w[4];
mat3 v_1(uint start_byte_offset) {
  return mat3(uintBitsToFloat(v.inner[(start_byte_offset / 16u)].xyz), uintBitsToFloat(v.inner[((16u + start_byte_offset) / 16u)].xyz), uintBitsToFloat(v.inner[((32u + start_byte_offset) / 16u)].xyz));
}
mat3[4] v_2(uint start_byte_offset) {
  mat3 a[4] = mat3[4](mat3(vec3(0.0f), vec3(0.0f), vec3(0.0f)), mat3(vec3(0.0f), vec3(0.0f), vec3(0.0f)), mat3(vec3(0.0f), vec3(0.0f), vec3(0.0f)), mat3(vec3(0.0f), vec3(0.0f), vec3(0.0f)));
  {
    uint idx = 0u;
    while(true) {
      uint v_3 = idx;
      if ((v_3 >= 4u)) {
        break;
      }
      a[v_3] = v_1((start_byte_offset + (v_3 * 48u)));
      {
        idx = (idx + 1u);
      }
    }
  }
  return a;
}
void f_inner(uint tint_local_index) {
  {
    uint idx = tint_local_index;
    while(true) {
      uint v_4 = idx;
      if ((v_4 >= 4u)) {
        break;
      }
      w[v_4] = mat3(vec3(0.0f), vec3(0.0f), vec3(0.0f));
      {
        idx = (idx + 1u);
      }
    }
  }
  barrier();
  w = v_2(0u);
  w[1] = v_1(96u);
  w[1][0] = uintBitsToFloat(v.inner[1u].xyz).zxy;
  uvec4 v_5 = v.inner[1u];
  w[1][0].x = uintBitsToFloat(v_5.x);
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  f_inner(gl_LocalInvocationIndex);
}

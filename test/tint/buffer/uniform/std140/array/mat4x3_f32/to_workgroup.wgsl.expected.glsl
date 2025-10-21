#version 310 es

layout(binding = 0, std140)
uniform u_block_1_ubo {
  uvec4 inner[16];
} v;
shared mat4x3 w[4];
mat4x3 v_1(uint start_byte_offset) {
  return mat4x3(uintBitsToFloat(v.inner[(start_byte_offset / 16u)].xyz), uintBitsToFloat(v.inner[((16u + start_byte_offset) / 16u)].xyz), uintBitsToFloat(v.inner[((32u + start_byte_offset) / 16u)].xyz), uintBitsToFloat(v.inner[((48u + start_byte_offset) / 16u)].xyz));
}
mat4x3[4] v_2(uint start_byte_offset) {
  mat4x3 a[4] = mat4x3[4](mat4x3(vec3(0.0f), vec3(0.0f), vec3(0.0f), vec3(0.0f)), mat4x3(vec3(0.0f), vec3(0.0f), vec3(0.0f), vec3(0.0f)), mat4x3(vec3(0.0f), vec3(0.0f), vec3(0.0f), vec3(0.0f)), mat4x3(vec3(0.0f), vec3(0.0f), vec3(0.0f), vec3(0.0f)));
  {
    uint v_3 = 0u;
    v_3 = 0u;
    while(true) {
      uint v_4 = v_3;
      if ((v_4 >= 4u)) {
        break;
      }
      a[v_4] = v_1((start_byte_offset + (v_4 * 64u)));
      {
        v_3 = (v_4 + 1u);
      }
      continue;
    }
  }
  return a;
}
void f_inner(uint tint_local_index) {
  {
    uint v_5 = 0u;
    v_5 = tint_local_index;
    while(true) {
      uint v_6 = v_5;
      if ((v_6 >= 4u)) {
        break;
      }
      w[v_6] = mat4x3(vec3(0.0f), vec3(0.0f), vec3(0.0f), vec3(0.0f));
      {
        v_5 = (v_6 + 1u);
      }
      continue;
    }
  }
  barrier();
  w = v_2(0u);
  w[1u] = v_1(128u);
  w[1u][0u] = uintBitsToFloat(v.inner[1u].xyz).zxy;
  uvec4 v_7 = v.inner[1u];
  w[1u][0u].x = uintBitsToFloat(v_7.x);
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  f_inner(gl_LocalInvocationIndex);
}

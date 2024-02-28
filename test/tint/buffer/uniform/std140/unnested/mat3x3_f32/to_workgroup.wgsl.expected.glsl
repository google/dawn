#version 310 es

shared mat3 w;
void tint_zero_workgroup_memory(uint local_idx) {
  if ((local_idx < 1u)) {
    w = mat3(vec3(0.0f), vec3(0.0f), vec3(0.0f));
  }
  barrier();
}

layout(binding = 0, std140) uniform u_block_ubo {
  mat3 inner;
} u;

void f(uint local_invocation_index) {
  tint_zero_workgroup_memory(local_invocation_index);
  w = u.inner;
  w[1] = u.inner[0];
  w[1] = u.inner[0].zxy;
  w[0][1] = u.inner[1][0];
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  f(gl_LocalInvocationIndex);
  return;
}

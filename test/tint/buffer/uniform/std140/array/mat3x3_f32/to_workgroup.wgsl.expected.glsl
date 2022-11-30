#version 310 es

layout(binding = 0, std140) uniform u_block_ubo {
  mat3 inner[4];
} u;

shared mat3 w[4];
void f(uint local_invocation_index) {
  {
    for(uint idx = local_invocation_index; (idx < 4u); idx = (idx + 1u)) {
      uint i = idx;
      w[i] = mat3(vec3(0.0f), vec3(0.0f), vec3(0.0f));
    }
  }
  barrier();
  w = u.inner;
  w[1] = u.inner[2];
  w[1][0] = u.inner[0][1].zxy;
  w[1][0].x = u.inner[0][1].x;
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  f(gl_LocalInvocationIndex);
  return;
}

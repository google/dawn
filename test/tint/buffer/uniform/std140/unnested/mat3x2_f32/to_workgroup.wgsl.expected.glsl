#version 310 es

layout(binding = 0, std140) uniform u_block_std140_ubo {
  vec2 inner_0;
  vec2 inner_1;
  vec2 inner_2;
} u;

shared mat3x2 w;
mat3x2 load_u_inner() {
  return mat3x2(u.inner_0, u.inner_1, u.inner_2);
}

void f(uint local_invocation_index) {
  {
    w = mat3x2(vec2(0.0f), vec2(0.0f), vec2(0.0f));
  }
  barrier();
  w = load_u_inner();
  w[1] = u.inner_0;
  w[1] = u.inner_0.yx;
  w[0][1] = u.inner_1[0u];
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  f(gl_LocalInvocationIndex);
  return;
}

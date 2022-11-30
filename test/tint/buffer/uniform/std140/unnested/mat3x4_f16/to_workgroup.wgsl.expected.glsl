#version 310 es
#extension GL_AMD_gpu_shader_half_float : require

layout(binding = 0, std140) uniform u_block_std140_ubo {
  f16vec4 inner_0;
  f16vec4 inner_1;
  f16vec4 inner_2;
} u;

shared f16mat3x4 w;
f16mat3x4 load_u_inner() {
  return f16mat3x4(u.inner_0, u.inner_1, u.inner_2);
}

void f(uint local_invocation_index) {
  {
    w = f16mat3x4(f16vec4(0.0hf), f16vec4(0.0hf), f16vec4(0.0hf));
  }
  barrier();
  w = load_u_inner();
  w[1] = u.inner_0;
  w[1] = u.inner_0.ywxz;
  w[0][1] = u.inner_1[0u];
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  f(gl_LocalInvocationIndex);
  return;
}

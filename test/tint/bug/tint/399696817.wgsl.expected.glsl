#version 310 es

shared int a;
void foo(uint v) {
  uvec4 v_1 = uvec4(24u, 16u, 8u, 0u);
  ivec4 v_2 = ivec4((uvec4(v) << v_1));
  ivec4 x = (v_2 >> uvec4(24u));
  uvec4 v_3 = uvec4(0u, 8u, 16u, 24u);
  uvec4 v_4 = (uvec4(v) >> v_3);
  uvec4 y = (v_4 & uvec4(255u));
  int z = atomicOr(a, 0);
}
void main_inner(uint tint_local_index) {
  if ((tint_local_index < 1u)) {
    atomicExchange(a, 0);
  }
  barrier();
  foo(1u);
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  main_inner(gl_LocalInvocationIndex);
}

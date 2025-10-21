#version 310 es


struct OuterS {
  uint a1[8];
};

layout(binding = 0, std140)
uniform uniforms_block_1_ubo {
  uvec4 inner[1];
} v_1;
uint f(uint i) {
  return (i + 1u);
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  OuterS s1 = OuterS(uint[8](0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u));
  vec3 v = vec3(0.0f);
  uvec4 v_2 = v_1.inner[0u];
  v[min(s1.a1[min(v_2.x, 7u)], 2u)] = 1.0f;
  uvec4 v_3 = v_1.inner[0u];
  v[min(f(s1.a1[min(v_3.x, 7u)]), 2u)] = 1.0f;
}

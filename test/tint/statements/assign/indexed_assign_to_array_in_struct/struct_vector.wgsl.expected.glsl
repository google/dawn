#version 310 es


struct OuterS {
  vec3 v1;
};

layout(binding = 0, std140)
uniform uniforms_block_1_ubo {
  uvec4 inner[1];
} v;
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  OuterS s1 = OuterS(vec3(0.0f));
  uvec4 v_1 = v.inner[0u];
  s1.v1[min(v_1.x, 2u)] = 1.0f;
}

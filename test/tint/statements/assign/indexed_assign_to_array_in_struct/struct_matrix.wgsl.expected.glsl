#version 310 es


struct Uniforms {
  uint i;
};

struct OuterS {
  mat2x4 m1;
};

layout(binding = 0, std140)
uniform uniforms_block_1_ubo {
  Uniforms inner;
} v;
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  OuterS s1 = OuterS(mat2x4(vec4(0.0f), vec4(0.0f)));
  uint v_1 = min(v.inner.i, 1u);
  s1.m1[v_1] = vec4(1.0f);
  uint v_2 = min(v.inner.i, 1u);
  s1.m1[v_2][min(v.inner.i, 3u)] = 1.0f;
}

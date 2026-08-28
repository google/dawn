#version 310 es


struct Results {
  float colorSamples[4];
};

layout(binding = 1, std430)
buffer results_block_1_ssbo {
  Results inner;
} v;
layout(binding = 0, std140)
uniform TintTextureUniformData_1_ubo {
  uvec4 metadata[1];
} v_1;
uniform highp sampler2DMS texture0;
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  uint v_2 = (v_1.metadata[0u].x - 1u);
  uint v_3 = min(uint(0), v_2);
  uvec2 v_4 = (uvec2(textureSize(texture0)) - uvec2(1u));
  ivec2 v_5 = ivec2(min(uvec2(ivec2(0)), v_4));
  v.inner.colorSamples[0u] = texelFetch(texture0, v_5, int(v_3)).x;
}

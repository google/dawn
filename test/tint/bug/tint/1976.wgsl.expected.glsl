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
  uint v_2 = min(0u, (v_1.metadata[0u].x - 1u));
  ivec2 v_3 = ivec2(min(uvec2(0u), (uvec2(textureSize(texture0)) - uvec2(1u))));
  v.inner.colorSamples[0u] = texelFetch(texture0, v_3, int(v_2)).x;
}

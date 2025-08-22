SKIP: INVALID

Error parsing GLSL shader:
ERROR: 0:18: 'variable indexing sampler array' : not supported for this version or the enabled extensions 
ERROR: 0:18: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



#version 310 es
precision highp float;
precision highp int;

layout(binding = 0, std140)
uniform f_index_block_ubo {
  uint inner;
} v;
layout(binding = 0, std140)
uniform f_TintTextureUniformData_ubo {
  uvec4 metadata[1];
} v_1;
uniform highp sampler2D f_sampled_textures[4];
void main() {
  uint v_2 = v.inner;
  uint v_3 = (v_1.metadata[((0u + v_2) / 4u)][((0u + v_2) % 4u)] - 1u);
  uint v_4 = min(uint(0), v_3);
  uvec2 v_5 = (uvec2(textureSize(f_sampled_textures[v_2], int(v_4))) - uvec2(1u));
  ivec2 v_6 = ivec2(min(uvec2(ivec2(0)), v_5));
  vec4 texture_load = texelFetch(f_sampled_textures[v_2], v_6, int(v_4));
}

tint executable returned error: exit status 1

SKIP: FAILED

Error parsing GLSL shader:
ERROR: 0:19: 'variable indexing sampler array' : not supported for this version or the enabled extensions 
ERROR: 0:19: '' : compilation terminated 
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
  uint v_3 = (0u + uint(v_2));
  uint v_4 = (v_1.metadata[(v_3 / 4u)][(v_3 % 4u)] - 1u);
  uint v_5 = min(uint(0), v_4);
  uvec2 v_6 = (uvec2(textureSize(f_sampled_textures[v_2], int(v_5))) - uvec2(1u));
  ivec2 v_7 = ivec2(min(uvec2(ivec2(0)), v_6));
  vec4 texture_load = texelFetch(f_sampled_textures[v_2], v_7, int(v_5));
}

tint executable returned error: exit status 1

SKIP: FAILED

Error parsing GLSL shader:
ERROR: 0:13: 'variable indexing sampler array' : not supported for this version or the enabled extensions 
ERROR: 0:13: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



#version 310 es
precision highp float;
precision highp int;

layout(binding = 0, std140)
uniform f_TintTextureUniformData_ubo {
  uvec4 metadata[1];
} v;
uniform highp sampler2D f_sampled_textures[4];
void do_texture_load(uint t_indices[1]) {
  uint v_1 = (v.metadata[((0u + t_indices[0u]) / 4u)][((0u + t_indices[0u]) % 4u)] - 1u);
  uint v_2 = min(uint(0), v_1);
  uvec2 v_3 = (uvec2(textureSize(f_sampled_textures[t_indices[0u]], int(v_2))) - uvec2(1u));
  ivec2 v_4 = ivec2(min(uvec2(ivec2(0)), v_3));
  vec4 texture_load = texelFetch(f_sampled_textures[t_indices[0u]], v_4, int(v_2));
}
void main() {
  do_texture_load(uint[1](uint(0)));
}

tint executable returned error: exit status 1

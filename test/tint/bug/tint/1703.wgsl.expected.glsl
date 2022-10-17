#version 310 es

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void unused_entry_point() {
  return;
}
vec4 my_global = vec4(0.0f, 0.0f, 0.0f, 0.0f);
layout(binding = 0, std140) uniform my_uniform_block_ubo {
  float inner;
} my_uniform;

uniform highp sampler2D my_texture_my_sampler;

void foo_member_initialize() {
  bvec2 vb2 = bvec2(false, false);
  vb2.x = (my_global.z != 0.0f);
  vb2.x = (my_uniform.inner == -1.0f);
  vb2 = bvec2((my_uniform.inner == -1.0f), false);
  if (vb2.x) {
    vec4 r = texture(my_texture_my_sampler, vec2(0.0f), 0.0f);
  }
}

void foo_default_initialize() {
  bvec2 vb2 = bvec2(false, false);
  vb2.x = (my_global.z != 0.0f);
  vb2.x = (my_uniform.inner == -1.0f);
  vb2 = bvec2(false);
  if (vb2.x) {
    vec4 r = texture(my_texture_my_sampler, vec2(0.0f), 0.0f);
  }
}


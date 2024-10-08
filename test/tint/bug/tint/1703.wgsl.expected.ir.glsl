#version 310 es

vec4 my_global = vec4(0.0f);
layout(binding = 0, std140)
uniform tint_symbol_1_1_ubo {
  float tint_symbol;
} v;
uniform highp sampler2D my_texture_my_sampler;
void foo_member_initialize() {
  bvec2 vb2 = bvec2(false);
  vb2[0u] = (my_global.z != 0.0f);
  vb2[0u] = (v.tint_symbol == -1.0f);
  vb2 = bvec2((v.tint_symbol == -1.0f), false);
  if (vb2.x) {
    vec4 r = texture(my_texture_my_sampler, vec2(0.0f), 0.0f);
  }
}
void foo_default_initialize() {
  bvec2 vb2 = bvec2(false);
  vb2[0u] = (my_global.z != 0.0f);
  vb2[0u] = (v.tint_symbol == -1.0f);
  vb2 = bvec2(false);
  if (vb2.x) {
    vec4 r = texture(my_texture_my_sampler, vec2(0.0f), 0.0f);
  }
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}

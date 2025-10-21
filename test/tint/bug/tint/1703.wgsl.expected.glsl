#version 310 es

vec4 my_global = vec4(0.0f);
layout(binding = 0, std140)
uniform my_uniform_block_1_ubo {
  uvec4 inner[1];
} v;
uniform highp sampler2D my_texture_my_sampler;
void foo_member_initialize() {
  bvec2 vb2 = bvec2(false);
  vb2.x = (my_global.z != 0.0f);
  uvec4 v_1 = v.inner[0u];
  vb2.x = (uintBitsToFloat(v_1.x) == -1.0f);
  uvec4 v_2 = v.inner[0u];
  vb2 = bvec2((uintBitsToFloat(v_2.x) == -1.0f), false);
  if (vb2.x) {
    vec4 r = texture(my_texture_my_sampler, vec2(0.0f), clamp(0.0f, -16.0f, 15.9899997711181640625f));
  }
}
void foo_default_initialize() {
  bvec2 vb2 = bvec2(false);
  vb2.x = (my_global.z != 0.0f);
  uvec4 v_3 = v.inner[0u];
  vb2.x = (uintBitsToFloat(v_3.x) == -1.0f);
  vb2 = bvec2(false);
  if (vb2.x) {
    vec4 r = texture(my_texture_my_sampler, vec2(0.0f), clamp(0.0f, -16.0f, 15.9899997711181640625f));
  }
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}

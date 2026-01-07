#version 310 es
precision highp float;
precision highp int;

layout(binding = 2, r32f) uniform highp image2D f_store;
uniform highp sampler2D f_tex_sam;
void main() {
  vec4 res = textureLodOffset(f_tex_sam, vec2(1.0f), 0.0f, ivec2(1));
  imageStore(f_store, ivec2(0), res);
}

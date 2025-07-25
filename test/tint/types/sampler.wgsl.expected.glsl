#version 310 es
precision highp float;
precision highp int;

uniform highp sampler2D f_t_s;
uniform highp sampler2DShadow f_d_sc;
void main() {
  vec4 a = texture(f_t_s, vec2(1.0f));
  vec4 b = textureGather(f_d_sc, vec2(1.0f), 1.0f);
}

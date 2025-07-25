#version 310 es
precision highp float;
precision highp int;

uniform highp sampler2D f_t_s;
void main() {
  vec4 res = textureGather(f_t_s, vec2(0.0f), 0);
}

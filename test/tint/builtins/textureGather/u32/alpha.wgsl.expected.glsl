#version 310 es
precision highp float;
precision highp int;

uniform highp usampler2D f_t_s;
void main() {
  uvec4 res = textureGather(f_t_s, vec2(0.0f), 3);
}

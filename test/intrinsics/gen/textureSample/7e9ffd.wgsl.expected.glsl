#version 310 es
precision mediump float;

uniform highp sampler2DArray arg_0;


void textureSample_7e9ffd() {
  float res = texture(arg_0, vec3(0.0f, 0.0f, float(1))).x;
}

void fragment_main() {
  textureSample_7e9ffd();
  return;
}
void main() {
  fragment_main();
}



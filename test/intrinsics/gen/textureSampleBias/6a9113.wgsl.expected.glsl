#version 310 es
precision mediump float;

uniform highp sampler2D arg_0;


void textureSampleBias_6a9113() {
  vec4 res = texture(arg_0, vec2(0.0f, 0.0f), 1.0f);
}

void fragment_main() {
  textureSampleBias_6a9113();
  return;
}
void main() {
  fragment_main();
}



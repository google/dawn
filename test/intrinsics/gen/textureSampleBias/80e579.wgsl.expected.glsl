#version 310 es
precision mediump float;

uniform highp sampler2DArray arg_0;


void textureSampleBias_80e579() {
  vec4 res = texture(arg_0, vec3(0.0f, 0.0f, float(1)), 1.0f);
}

void fragment_main() {
  textureSampleBias_80e579();
  return;
}
void main() {
  fragment_main();
}



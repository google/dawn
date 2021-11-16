#version 310 es
precision mediump float;

uniform highp sampler3D arg_0;


void textureSampleBias_d3fa1b() {
  vec4 res = texture(arg_0, vec3(0.0f, 0.0f, 0.0f), 1.0f);
}

void fragment_main() {
  textureSampleBias_d3fa1b();
  return;
}
void main() {
  fragment_main();
}



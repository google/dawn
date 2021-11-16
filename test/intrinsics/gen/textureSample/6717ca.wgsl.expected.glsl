#version 310 es
precision mediump float;

uniform highp sampler2DArray arg_0;


void textureSample_6717ca() {
  vec4 res = texture(arg_0, vec3(0.0f, 0.0f, float(1)));
}

void fragment_main() {
  textureSample_6717ca();
  return;
}
void main() {
  fragment_main();
}



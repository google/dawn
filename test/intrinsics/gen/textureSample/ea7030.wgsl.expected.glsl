#version 310 es
precision mediump float;

uniform highp samplerCube arg_0;


void textureSample_ea7030() {
  float res = texture(arg_0, vec3(0.0f, 0.0f, 0.0f)).x;
}

void fragment_main() {
  textureSample_ea7030();
  return;
}
void main() {
  fragment_main();
}



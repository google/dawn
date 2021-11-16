#version 310 es
precision mediump float;

uniform highp samplerCube arg_0;


void textureSample_e53267() {
  vec4 res = texture(arg_0, vec3(0.0f, 0.0f, 0.0f));
}

void fragment_main() {
  textureSample_e53267();
  return;
}
void main() {
  fragment_main();
}



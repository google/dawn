#version 310 es
precision mediump float;

uniform highp samplerCube arg_0_arg_1;

void textureSampleBias_53b9f7() {
  vec4 res = texture(arg_0_arg_1, vec3(0.0f, 0.0f, 0.0f), 1.0f);
}

void fragment_main() {
  textureSampleBias_53b9f7();
}

void main() {
  fragment_main();
  return;
}

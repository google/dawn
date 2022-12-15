SKIP: FAILED

#version 310 es
precision mediump float;

uniform highp samplerCubeArrayShadow arg_0_arg_1;

void textureSampleCompare_a3ca7e() {
  float res = texture(arg_0_arg_1, vec4(vec3(1.0f), float(1)), 1.0f);
}

void fragment_main() {
  textureSampleCompare_a3ca7e();
}

void main() {
  fragment_main();
  return;
}
Error parsing GLSL shader:
ERROR: 0:4: 'samplerCubeArrayShadow' : Reserved word. 
ERROR: 0:4: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




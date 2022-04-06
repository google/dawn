SKIP: FAILED

#version 310 es
precision mediump float;

uniform highp sampler1D arg_0_arg_1;

void textureSample_6e64fb() {
  vec4 res = texture(arg_0_arg_1, 1.0f);
}

void fragment_main() {
  textureSample_6e64fb();
}

void main() {
  fragment_main();
  return;
}
Error parsing GLSL shader:
ERROR: 0:4: 'sampler1D' : Reserved word. 
ERROR: 0:4: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




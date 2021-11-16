SKIP: FAILED

#version 310 es
precision mediump float;

uniform highp sampler2DArray arg_0;


void textureSample_8522e7() {
  float res = texture(arg_0, vec3(0.0f, 0.0f, float(1)), ivec2(0, 0)).x;
}

void fragment_main() {
  textureSample_8522e7();
  return;
}
void main() {
  fragment_main();
}


Error parsing GLSL shader:
ERROR: 0:8: 'texture' : no matching overloaded function found 
ERROR: 0:8: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




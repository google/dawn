SKIP: FAILED

#version 310 es
precision mediump float;

uniform highp samplerCubeArray arg_0;


void textureSample_c2f4e8() {
  float res = texture(arg_0, vec4(0.0f, 0.0f, 0.0f, float(1))).x;
}

void fragment_main() {
  textureSample_c2f4e8();
  return;
}
void main() {
  fragment_main();
}


Error parsing GLSL shader:
ERROR: 0:4: 'samplerCubeArray' : Reserved word. 
ERROR: 0:4: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




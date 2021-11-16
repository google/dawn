SKIP: FAILED

#version 310 es
precision mediump float;

uniform highp samplerCubeArray arg_0;


void textureSampleBias_eed7c4() {
  vec4 res = texture(arg_0, vec4(0.0f, 0.0f, 0.0f, float(1)), 1.0f);
}

void fragment_main() {
  textureSampleBias_eed7c4();
  return;
}
void main() {
  fragment_main();
}


Error parsing GLSL shader:
ERROR: 0:4: 'samplerCubeArray' : Reserved word. 
ERROR: 0:4: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




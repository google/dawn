SKIP: FAILED

#version 310 es
precision mediump float;

uniform highp samplerCube arg_0;


void textureSampleCompare_63fb83() {
  float res = texture(arg_0, vec3(0.0f, 0.0f, 0.0f), 1.0f);
}

void fragment_main() {
  textureSampleCompare_63fb83();
  return;
}
void main() {
  fragment_main();
}


Error parsing GLSL shader:
ERROR: 0:8: '=' :  cannot convert from ' global highp 4-component vector of float' to ' temp mediump float'
ERROR: 0:8: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




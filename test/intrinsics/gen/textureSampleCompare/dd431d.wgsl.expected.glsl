SKIP: FAILED

#version 310 es
precision mediump float;

uniform highp sampler2DArray arg_0;


void textureSampleCompare_dd431d() {
  float res = texture(arg_0, vec3(0.0f, 0.0f, float(1)), 1.0f);
}

void fragment_main() {
  textureSampleCompare_dd431d();
  return;
}
void main() {
  fragment_main();
}


Error parsing GLSL shader:
ERROR: 0:8: '=' :  cannot convert from ' global highp 4-component vector of float' to ' temp mediump float'
ERROR: 0:8: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




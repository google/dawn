SKIP: FAILED

#version 310 es
precision mediump float;

uniform highp sampler2DArray arg_0;


void textureSample_02aa9b() {
  vec4 res = texture(arg_0, vec3(0.0f, 0.0f, float(1)), ivec2(0, 0));
}

void fragment_main() {
  textureSample_02aa9b();
  return;
}
void main() {
  fragment_main();
}


Error parsing GLSL shader:
ERROR: 0:8: 'texture' : no matching overloaded function found 
ERROR: 0:8: '=' :  cannot convert from ' const float' to ' temp mediump 4-component vector of float'
ERROR: 0:8: '' : compilation terminated 
ERROR: 3 compilation errors.  No code generated.




SKIP: FAILED

#version 310 es
precision mediump float;

uniform highp sampler2D arg_0;


void textureSampleBias_81c19a() {
  vec4 res = texture(arg_0, vec2(0.0f, 0.0f), 1.0f, ivec2(0, 0));
}

void fragment_main() {
  textureSampleBias_81c19a();
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




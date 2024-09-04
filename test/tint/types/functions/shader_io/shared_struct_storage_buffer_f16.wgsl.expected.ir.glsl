SKIP: FAILED

#version 310 es
#extension GL_AMD_gpu_shader_half_float: require

struct S {
  float f;
  uint u;
  vec4 v;
  float16_t x;
  f16vec3 y;
};
precision highp float;
precision highp int;


S tint_symbol;
void tint_store_and_preserve_padding(inout S target, S value_param) {
  target.f = value_param.f;
  target.u = value_param.u;
  target.v = value_param.v;
  target.x = value_param.x;
  target.y = value_param.y;
}
void main(S tint_symbol_1) {
  float f = tint_symbol_1.f;
  uint u = tint_symbol_1.u;
  vec4 v = tint_symbol_1.v;
  float16_t x = tint_symbol_1.x;
  f16vec3 y = tint_symbol_1.y;
  tint_store_and_preserve_padding(tint_symbol, tint_symbol_1);
}
error: Error parsing GLSL shader:
ERROR: 0:5: 'float' : type requires declaration of default precision qualifier 
ERROR: 0:5: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1

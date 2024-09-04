SKIP: FAILED

#version 310 es

struct frexp_result_f32 {
  float fract;
  int exp;
};

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  float runtime_in = 1.25f;
  frexp_result_f32 res = frexp_result_f32(0.625f, 1);
  res = frexp(runtime_in);
  res = frexp_result_f32(0.625f, 1);
  float tint_symbol_1 = res.fract;
  int tint_symbol_2 = res.exp;
}
error: Error parsing GLSL shader:
ERROR: 0:12: 'frexp' : no matching overloaded function found 
ERROR: 0:12: 'assign' :  cannot convert from ' const float' to ' temp structure{ global highp float fract,  global highp int exp}'
ERROR: 0:12: '' : compilation terminated 
ERROR: 3 compilation errors.  No code generated.




tint executable returned error: exit status 1

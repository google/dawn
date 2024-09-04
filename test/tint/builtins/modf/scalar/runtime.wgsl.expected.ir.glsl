SKIP: FAILED

#version 310 es

struct modf_result_f32 {
  float fract;
  float whole;
};

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  float tint_symbol_1 = 1.25f;
  modf_result_f32 res = modf(tint_symbol_1);
  float tint_symbol_2 = res.fract;
  float whole = res.whole;
}
error: Error parsing GLSL shader:
ERROR: 0:11: 'modf' : no matching overloaded function found 
ERROR: 0:11: '=' :  cannot convert from ' const float' to ' temp structure{ global highp float fract,  global highp float whole}'
ERROR: 0:11: '' : compilation terminated 
ERROR: 3 compilation errors.  No code generated.




tint executable returned error: exit status 1

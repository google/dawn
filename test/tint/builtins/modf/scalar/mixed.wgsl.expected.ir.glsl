SKIP: FAILED

#version 310 es


struct modf_result_f32 {
  float fract;
  float whole;
};

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  float runtime_in = 1.25f;
  modf_result_f32 res = modf_result_f32(0.25f, 1.0f);
  res = modf(runtime_in);
  res = modf_result_f32(0.25f, 1.0f);
  float tint_symbol_1 = res.fract;
  float whole = res.whole;
}
error: Error parsing GLSL shader:
ERROR: 0:13: 'modf' : no matching overloaded function found 
ERROR: 0:13: 'assign' :  cannot convert from ' const float' to ' temp structure{ global highp float fract,  global highp float whole}'
ERROR: 0:13: '' : compilation terminated 
ERROR: 3 compilation errors.  No code generated.




tint executable returned error: exit status 1

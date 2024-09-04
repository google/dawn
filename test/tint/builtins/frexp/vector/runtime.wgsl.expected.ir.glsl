SKIP: FAILED

#version 310 es

struct frexp_result_vec2_f32 {
  vec2 fract;
  ivec2 exp;
};

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  vec2 tint_symbol_1 = vec2(1.25f, 3.75f);
  frexp_result_vec2_f32 res = frexp(tint_symbol_1);
  vec2 tint_symbol_2 = res.fract;
  ivec2 tint_symbol_3 = res.exp;
}
error: Error parsing GLSL shader:
ERROR: 0:11: 'frexp' : no matching overloaded function found 
ERROR: 0:11: '=' :  cannot convert from ' const float' to ' temp structure{ global highp 2-component vector of float fract,  global highp 2-component vector of int exp}'
ERROR: 0:11: '' : compilation terminated 
ERROR: 3 compilation errors.  No code generated.




tint executable returned error: exit status 1

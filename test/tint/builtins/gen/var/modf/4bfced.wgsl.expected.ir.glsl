SKIP: FAILED

#version 310 es
precision highp float;
precision highp int;


struct modf_result_vec4_f32 {
  vec4 fract;
  vec4 whole;
};

void modf_4bfced() {
  vec4 arg_0 = vec4(-1.5f);
  modf_result_vec4_f32 res = modf(arg_0);
}
void main() {
  modf_4bfced();
}
error: Error parsing GLSL shader:
ERROR: 0:13: 'modf' : no matching overloaded function found 
ERROR: 0:13: '=' :  cannot convert from ' const float' to ' temp structure{ global highp 4-component vector of float fract,  global highp 4-component vector of float whole}'
ERROR: 0:13: '' : compilation terminated 
ERROR: 3 compilation errors.  No code generated.



#version 310 es


struct modf_result_vec4_f32 {
  vec4 fract;
  vec4 whole;
};

void modf_4bfced() {
  vec4 arg_0 = vec4(-1.5f);
  modf_result_vec4_f32 res = modf(arg_0);
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  modf_4bfced();
}
error: Error parsing GLSL shader:
ERROR: 0:11: 'modf' : no matching overloaded function found 
ERROR: 0:11: '=' :  cannot convert from ' const float' to ' temp structure{ global highp 4-component vector of float fract,  global highp 4-component vector of float whole}'
ERROR: 0:11: '' : compilation terminated 
ERROR: 3 compilation errors.  No code generated.



#version 310 es


struct modf_result_vec4_f32 {
  vec4 fract;
  vec4 whole;
};

struct VertexOutput {
  vec4 pos;
};

void modf_4bfced() {
  vec4 arg_0 = vec4(-1.5f);
  modf_result_vec4_f32 res = modf(arg_0);
}
VertexOutput vertex_main_inner() {
  VertexOutput tint_symbol = VertexOutput(vec4(0.0f));
  tint_symbol.pos = vec4(0.0f);
  modf_4bfced();
  return tint_symbol;
}
void main() {
  gl_Position = vertex_main_inner().pos;
  gl_Position[1u] = -(gl_Position.y);
  gl_Position[2u] = ((2.0f * gl_Position.z) - gl_Position.w);
  gl_PointSize = 1.0f;
}
error: Error parsing GLSL shader:
ERROR: 0:15: 'modf' : no matching overloaded function found 
ERROR: 0:15: '=' :  cannot convert from ' const float' to ' temp structure{ global highp 4-component vector of float fract,  global highp 4-component vector of float whole}'
ERROR: 0:15: '' : compilation terminated 
ERROR: 3 compilation errors.  No code generated.




tint executable returned error: exit status 1

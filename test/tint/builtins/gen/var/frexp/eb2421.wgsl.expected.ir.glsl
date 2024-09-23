SKIP: FAILED

#version 310 es
precision highp float;
precision highp int;


struct frexp_result_vec2_f32 {
  vec2 fract;
  ivec2 exp;
};

void frexp_eb2421() {
  vec2 arg_0 = vec2(1.0f);
  frexp_result_vec2_f32 res = frexp(arg_0);
}
void main() {
  frexp_eb2421();
}
error: Error parsing GLSL shader:
ERROR: 0:13: 'frexp' : no matching overloaded function found 
ERROR: 0:13: '=' :  cannot convert from ' const float' to ' temp structure{ global highp 2-component vector of float fract,  global highp 2-component vector of int exp}'
ERROR: 0:13: '' : compilation terminated 
ERROR: 3 compilation errors.  No code generated.



#version 310 es


struct frexp_result_vec2_f32 {
  vec2 fract;
  ivec2 exp;
};

void frexp_eb2421() {
  vec2 arg_0 = vec2(1.0f);
  frexp_result_vec2_f32 res = frexp(arg_0);
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  frexp_eb2421();
}
error: Error parsing GLSL shader:
ERROR: 0:11: 'frexp' : no matching overloaded function found 
ERROR: 0:11: '=' :  cannot convert from ' const float' to ' temp structure{ global highp 2-component vector of float fract,  global highp 2-component vector of int exp}'
ERROR: 0:11: '' : compilation terminated 
ERROR: 3 compilation errors.  No code generated.



#version 310 es


struct frexp_result_vec2_f32 {
  vec2 fract;
  ivec2 exp;
};

struct VertexOutput {
  vec4 pos;
};

void frexp_eb2421() {
  vec2 arg_0 = vec2(1.0f);
  frexp_result_vec2_f32 res = frexp(arg_0);
}
VertexOutput vertex_main_inner() {
  VertexOutput tint_symbol = VertexOutput(vec4(0.0f));
  tint_symbol.pos = vec4(0.0f);
  frexp_eb2421();
  return tint_symbol;
}
void main() {
  gl_Position = vertex_main_inner().pos;
  gl_Position[1u] = -(gl_Position.y);
  gl_Position[2u] = ((2.0f * gl_Position.z) - gl_Position.w);
  gl_PointSize = 1.0f;
}
error: Error parsing GLSL shader:
ERROR: 0:15: 'frexp' : no matching overloaded function found 
ERROR: 0:15: '=' :  cannot convert from ' const float' to ' temp structure{ global highp 2-component vector of float fract,  global highp 2-component vector of int exp}'
ERROR: 0:15: '' : compilation terminated 
ERROR: 3 compilation errors.  No code generated.




tint executable returned error: exit status 1

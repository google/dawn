SKIP: FAILED

#version 310 es
precision highp float;
precision highp int;

layout(binding = 0, std430)
buffer tint_symbol_1_1_ssbo {
  int tint_symbol;
} v;
int select_3c25ce() {
  bvec3 arg_0 = bvec3(true);
  bvec3 arg_1 = bvec3(true);
  bool arg_2 = true;
  bvec3 v_1 = arg_0;
  bvec3 v_2 = arg_1;
  bool v_3 = arg_2;
  bool v_4 = ((v_3.x) ? (v_2.x) : (v_1.x));
  bool v_5 = ((v_3.y) ? (v_2.y) : (v_1.y));
  bvec3 res = bvec3(v_4, v_5, ((v_3.z) ? (v_2.z) : (v_1.z)));
  return ((all((res == bvec3(false)))) ? (1) : (0));
}
void main() {
  v.tint_symbol = select_3c25ce();
}
error: Error parsing GLSL shader:
ERROR: 0:16: 'scalar swizzle' : not supported with this profile: es
ERROR: 0:16: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



#version 310 es

layout(binding = 0, std430)
buffer tint_symbol_1_1_ssbo {
  int tint_symbol;
} v;
int select_3c25ce() {
  bvec3 arg_0 = bvec3(true);
  bvec3 arg_1 = bvec3(true);
  bool arg_2 = true;
  bvec3 v_1 = arg_0;
  bvec3 v_2 = arg_1;
  bool v_3 = arg_2;
  bool v_4 = ((v_3.x) ? (v_2.x) : (v_1.x));
  bool v_5 = ((v_3.y) ? (v_2.y) : (v_1.y));
  bvec3 res = bvec3(v_4, v_5, ((v_3.z) ? (v_2.z) : (v_1.z)));
  return ((all((res == bvec3(false)))) ? (1) : (0));
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  v.tint_symbol = select_3c25ce();
}
error: Error parsing GLSL shader:
ERROR: 0:14: 'scalar swizzle' : not supported with this profile: es
ERROR: 0:14: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



#version 310 es


struct VertexOutput {
  vec4 pos;
  int prevent_dce;
};

layout(location = 0) flat out int vertex_main_loc0_Output;
int select_3c25ce() {
  bvec3 arg_0 = bvec3(true);
  bvec3 arg_1 = bvec3(true);
  bool arg_2 = true;
  bvec3 v = arg_0;
  bvec3 v_1 = arg_1;
  bool v_2 = arg_2;
  bool v_3 = ((v_2.x) ? (v_1.x) : (v.x));
  bool v_4 = ((v_2.y) ? (v_1.y) : (v.y));
  bvec3 res = bvec3(v_3, v_4, ((v_2.z) ? (v_1.z) : (v.z)));
  return ((all((res == bvec3(false)))) ? (1) : (0));
}
VertexOutput vertex_main_inner() {
  VertexOutput tint_symbol = VertexOutput(vec4(0.0f), 0);
  tint_symbol.pos = vec4(0.0f);
  tint_symbol.prevent_dce = select_3c25ce();
  return tint_symbol;
}
void main() {
  VertexOutput v_5 = vertex_main_inner();
  gl_Position = v_5.pos;
  gl_Position[1u] = -(gl_Position.y);
  gl_Position[2u] = ((2.0f * gl_Position.z) - gl_Position.w);
  vertex_main_loc0_Output = v_5.prevent_dce;
  gl_PointSize = 1.0f;
}
error: Error parsing GLSL shader:
ERROR: 0:17: 'scalar swizzle' : not supported with this profile: es
ERROR: 0:17: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1

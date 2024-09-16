SKIP: FAILED

#version 310 es
precision highp float;
precision highp int;

void select_3a14be() {
  bool arg_2 = true;
  bool v = arg_2;
  int v_1 = ((v.x) ? (ivec2(1).x) : (ivec2(1).x));
  ivec2 res = ivec2(v_1, ((v.y) ? (ivec2(1).y) : (ivec2(1).y)));
}
void main() {
  select_3a14be();
}
error: Error parsing GLSL shader:
ERROR: 0:8: 'scalar swizzle' : not supported with this profile: es
ERROR: 0:8: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



#version 310 es

void select_3a14be() {
  bool arg_2 = true;
  bool v = arg_2;
  int v_1 = ((v.x) ? (ivec2(1).x) : (ivec2(1).x));
  ivec2 res = ivec2(v_1, ((v.y) ? (ivec2(1).y) : (ivec2(1).y)));
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  select_3a14be();
}
error: Error parsing GLSL shader:
ERROR: 0:6: 'scalar swizzle' : not supported with this profile: es
ERROR: 0:6: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



#version 310 es


struct VertexOutput {
  vec4 pos;
};

void select_3a14be() {
  bool arg_2 = true;
  bool v = arg_2;
  int v_1 = ((v.x) ? (ivec2(1).x) : (ivec2(1).x));
  ivec2 res = ivec2(v_1, ((v.y) ? (ivec2(1).y) : (ivec2(1).y)));
}
VertexOutput vertex_main_inner() {
  VertexOutput tint_symbol = VertexOutput(vec4(0.0f));
  tint_symbol.pos = vec4(0.0f);
  select_3a14be();
  return tint_symbol;
}
void main() {
  gl_Position = vertex_main_inner().pos;
  gl_Position[1u] = -(gl_Position.y);
  gl_Position[2u] = ((2.0f * gl_Position.z) - gl_Position.w);
  gl_PointSize = 1.0f;
}
error: Error parsing GLSL shader:
ERROR: 0:11: 'scalar swizzle' : not supported with this profile: es
ERROR: 0:11: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1

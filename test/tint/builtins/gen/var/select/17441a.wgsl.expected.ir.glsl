SKIP: FAILED

#version 310 es
precision highp float;
precision highp int;

void select_17441a() {
  bool arg_2 = true;
  bool v = arg_2;
  float v_1 = ((v.x) ? (vec4(1.0f).x) : (vec4(1.0f).x));
  float v_2 = ((v.y) ? (vec4(1.0f).y) : (vec4(1.0f).y));
  float v_3 = ((v.z) ? (vec4(1.0f).z) : (vec4(1.0f).z));
  vec4 res = vec4(v_1, v_2, v_3, ((v.w) ? (vec4(1.0f).w) : (vec4(1.0f).w)));
}
void main() {
  select_17441a();
}
error: Error parsing GLSL shader:
ERROR: 0:8: 'scalar swizzle' : not supported with this profile: es
ERROR: 0:8: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



#version 310 es

void select_17441a() {
  bool arg_2 = true;
  bool v = arg_2;
  float v_1 = ((v.x) ? (vec4(1.0f).x) : (vec4(1.0f).x));
  float v_2 = ((v.y) ? (vec4(1.0f).y) : (vec4(1.0f).y));
  float v_3 = ((v.z) ? (vec4(1.0f).z) : (vec4(1.0f).z));
  vec4 res = vec4(v_1, v_2, v_3, ((v.w) ? (vec4(1.0f).w) : (vec4(1.0f).w)));
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  select_17441a();
}
error: Error parsing GLSL shader:
ERROR: 0:6: 'scalar swizzle' : not supported with this profile: es
ERROR: 0:6: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



#version 310 es


struct VertexOutput {
  vec4 pos;
};

void select_17441a() {
  bool arg_2 = true;
  bool v = arg_2;
  float v_1 = ((v.x) ? (vec4(1.0f).x) : (vec4(1.0f).x));
  float v_2 = ((v.y) ? (vec4(1.0f).y) : (vec4(1.0f).y));
  float v_3 = ((v.z) ? (vec4(1.0f).z) : (vec4(1.0f).z));
  vec4 res = vec4(v_1, v_2, v_3, ((v.w) ? (vec4(1.0f).w) : (vec4(1.0f).w)));
}
VertexOutput vertex_main_inner() {
  VertexOutput tint_symbol = VertexOutput(vec4(0.0f));
  tint_symbol.pos = vec4(0.0f);
  select_17441a();
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

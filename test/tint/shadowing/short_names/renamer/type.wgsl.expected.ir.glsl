SKIP: FAILED

#version 310 es


struct vec4f {
  int i;
};

vec4 tint_symbol_inner(uint VertexIndex) {
  vec4f s = vec4f(1);
  float f = float(s.i);
  bool b = bool(f);
  float v = ((b.x) ? (vec4(1.0f).x) : (vec4(0.0f).x));
  float v_1 = ((b.y) ? (vec4(1.0f).y) : (vec4(0.0f).y));
  float v_2 = ((b.z) ? (vec4(1.0f).z) : (vec4(0.0f).z));
  return vec4(v, v_1, v_2, ((b.w) ? (vec4(1.0f).w) : (vec4(0.0f).w)));
}
void main() {
  gl_Position = tint_symbol_inner(gl_VertexID);
  gl_Position[1u] = -(gl_Position.y);
  gl_Position[2u] = ((2.0f * gl_Position.z) - gl_Position.w);
  gl_PointSize = 1.0f;
}
error: Error parsing GLSL shader:
ERROR: 0:12: 'scalar swizzle' : not supported with this profile: es
ERROR: 0:12: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1

SKIP: FAILED

#version 310 es

int vec4f() {
  return 0;
}
float vec2f(int i) {
  return float(i);
}
bool vec2i(float f) {
  return bool(f);
}
vec4 tint_symbol_inner(uint VertexIndex) {
  bool v = vec2i(vec2f(vec4f()));
  float v_1 = ((v.x) ? (vec4(1.0f).x) : (vec4(0.0f).x));
  float v_2 = ((v.y) ? (vec4(1.0f).y) : (vec4(0.0f).y));
  float v_3 = ((v.z) ? (vec4(1.0f).z) : (vec4(0.0f).z));
  return vec4(v_1, v_2, v_3, ((v.w) ? (vec4(1.0f).w) : (vec4(0.0f).w)));
}
void main() {
  gl_Position = tint_symbol_inner(gl_VertexID);
  gl_Position[1u] = -(gl_Position.y);
  gl_Position[2u] = ((2.0f * gl_Position.z) - gl_Position.w);
  gl_PointSize = 1.0f;
}
error: Error parsing GLSL shader:
ERROR: 0:14: 'scalar swizzle' : not supported with this profile: es
ERROR: 0:14: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1

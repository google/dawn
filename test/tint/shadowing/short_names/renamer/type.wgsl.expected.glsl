#version 310 es

struct vec4f {
  int i;
};

vec4 tint_symbol(uint VertexIndex) {
  vec4f s = vec4f(1);
  float f = float(s.i);
  bool b = bool(f);
  return (b ? vec4(1.0f) : vec4(0.0f));
}

void main() {
  gl_PointSize = 1.0;
  vec4 inner_result = tint_symbol(uint(gl_VertexID));
  gl_Position = inner_result;
  gl_Position.y = -(gl_Position.y);
  gl_Position.z = ((2.0f * gl_Position.z) - gl_Position.w);
  return;
}

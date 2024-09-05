#version 310 es


struct Out {
  vec4 pos;
};

Out tint_symbol_inner() {
  return Out(vec4(0.0f));
}
void main() {
  gl_Position = tint_symbol_inner().pos;
  gl_Position[1u] = -(gl_Position.y);
  gl_Position[2u] = ((2.0f * gl_Position.z) - gl_Position.w);
  gl_PointSize = 1.0f;
}

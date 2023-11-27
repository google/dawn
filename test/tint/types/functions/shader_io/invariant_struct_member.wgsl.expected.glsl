#version 310 es

struct Out {
  vec4 pos;
};

Out tint_symbol() {
  Out tint_symbol_1 = Out(vec4(0.0f));
  return tint_symbol_1;
}

void main() {
  gl_PointSize = 1.0;
  Out inner_result = tint_symbol();
  gl_Position = inner_result.pos;
  gl_Position.y = -(gl_Position.y);
  gl_Position.z = ((2.0f * gl_Position.z) - gl_Position.w);
  return;
}

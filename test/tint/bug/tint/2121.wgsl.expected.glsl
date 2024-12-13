#version 310 es


struct VSOut {
  vec4 pos;
};

void foo(inout VSOut v) {
  vec4 pos = vec4(1.0f, 2.0f, 3.0f, 4.0f);
  v.pos = pos;
}
VSOut main_inner() {
  VSOut v_1 = VSOut(vec4(0.0f));
  foo(v_1);
  return v_1;
}
void main() {
  gl_Position = main_inner().pos;
  gl_Position.y = -(gl_Position.y);
  gl_Position.z = ((2.0f * gl_Position.z) - gl_Position.w);
  gl_PointSize = 1.0f;
}

#version 310 es


struct main_out {
  vec4 member_0;
};

vec4 v = vec4(0.0f);
void main_1() {
  v = vec4(0.0f);
}
main_out main_inner() {
  main_1();
  return main_out(v);
}
void main() {
  gl_Position = main_inner().member_0;
  gl_Position.y = -(gl_Position.y);
  gl_Position.z = ((2.0f * gl_Position.z) - gl_Position.w);
  gl_PointSize = 1.0f;
}

#version 310 es


struct Out {
  vec4 pos;
};

Out main_inner() {
  return Out(vec4(0.0f));
}
void main() {
  gl_Position = main_inner().pos;
  gl_Position.y = -(gl_Position.y);
  gl_Position.z = ((2.0f * gl_Position.z) - gl_Position.w);
  gl_PointSize = 1.0f;
}

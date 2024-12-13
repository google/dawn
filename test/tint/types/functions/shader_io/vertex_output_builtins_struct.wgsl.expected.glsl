#version 310 es


struct VertexOutputs {
  vec4 position;
};

VertexOutputs main_inner() {
  return VertexOutputs(vec4(1.0f, 2.0f, 3.0f, 4.0f));
}
void main() {
  gl_Position = main_inner().position;
  gl_Position.y = -(gl_Position.y);
  gl_Position.z = ((2.0f * gl_Position.z) - gl_Position.w);
  gl_PointSize = 1.0f;
}

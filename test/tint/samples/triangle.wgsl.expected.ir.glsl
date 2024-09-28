#version 310 es

vec4 vtx_main_inner(uint VertexIndex) {
  return vec4(vec2[3](vec2(0.0f, 0.5f), vec2(-0.5f), vec2(0.5f, -0.5f))[VertexIndex], 0.0f, 1.0f);
}
void main() {
  gl_Position = vtx_main_inner(uint(gl_VertexID));
  gl_Position[1u] = -(gl_Position.y);
  gl_Position[2u] = ((2.0f * gl_Position.z) - gl_Position.w);
  gl_PointSize = 1.0f;
}
#version 310 es
precision highp float;
precision highp int;

layout(location = 0) out vec4 frag_main_loc0_Output;
vec4 frag_main_inner() {
  return vec4(1.0f, 0.0f, 0.0f, 1.0f);
}
void main() {
  frag_main_loc0_Output = frag_main_inner();
}

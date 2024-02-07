#version 310 es

vec4 vtx_main(uint VertexIndex) {
  vec2 tint_symbol[3] = vec2[3](vec2(0.0f, 0.5f), vec2(-0.5f), vec2(0.5f, -0.5f));
  return vec4(tint_symbol[VertexIndex], 0.0f, 1.0f);
}

void main() {
  gl_PointSize = 1.0;
  vec4 inner_result = vtx_main(uint(gl_VertexID));
  gl_Position = inner_result;
  gl_Position.y = -(gl_Position.y);
  gl_Position.z = ((2.0f * gl_Position.z) - gl_Position.w);
  return;
}
#version 310 es
precision highp float;
precision highp int;

layout(location = 0) out vec4 value;
vec4 frag_main() {
  return vec4(1.0f, 0.0f, 0.0f, 1.0f);
}

void main() {
  vec4 inner_result = frag_main();
  value = inner_result;
  return;
}

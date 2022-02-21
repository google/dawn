#version 310 es

const vec2 pos[3] = vec2[3](vec2(0.0f, 0.5f), vec2(-0.5f, -0.5f), vec2(0.5f, -0.5f));
vec4 vtx_main(uint VertexIndex) {
  return vec4(pos[VertexIndex], 0.0f, 1.0f);
}

void main() {
  vec4 inner_result = vtx_main(uint(gl_VertexID));
  gl_Position = inner_result;
  gl_Position.y = -(gl_Position.y);
  gl_Position.z = ((2.0f * gl_Position.z) - gl_Position.w);
  return;
}
#version 310 es
precision mediump float;

layout(location = 0) out vec4 value;
vec4 frag_main() {
  return vec4(1.0f, 0.0f, 0.0f, 1.0f);
}

void main() {
  vec4 inner_result = frag_main();
  value = inner_result;
  return;
}

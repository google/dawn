#version 310 es

vec4 vs_main(uint in_vertex_index) {
  vec4 tint_symbol[3] = vec4[3](vec4(0.0f, 0.0f, 0.0f, 1.0f), vec4(0.0f, 1.0f, 0.0f, 1.0f), vec4(1.0f, 1.0f, 0.0f, 1.0f));
  return tint_symbol[in_vertex_index];
}

void main() {
  gl_PointSize = 1.0;
  vec4 inner_result = vs_main(uint(gl_VertexID));
  gl_Position = inner_result;
  gl_Position.y = -(gl_Position.y);
  gl_Position.z = ((2.0f * gl_Position.z) - gl_Position.w);
  return;
}

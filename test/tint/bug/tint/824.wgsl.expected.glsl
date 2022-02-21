#version 310 es

layout(location = 0) out vec4 color_1;
struct Output {
  vec4 Position;
  vec4 color;
};

Output tint_symbol(uint VertexIndex, uint InstanceIndex) {
  vec2 zv[4] = vec2[4](vec2(0.200000003f, 0.200000003f), vec2(0.300000012f, 0.300000012f), vec2(-0.100000001f, -0.100000001f), vec2(1.100000024f, 1.100000024f));
  float z = zv[InstanceIndex].x;
  Output tint_symbol_1 = Output(vec4(0.0f, 0.0f, 0.0f, 0.0f), vec4(0.0f, 0.0f, 0.0f, 0.0f));
  tint_symbol_1.Position = vec4(0.5f, 0.5f, z, 1.0f);
  vec4 colors[4] = vec4[4](vec4(1.0f, 0.0f, 0.0f, 1.0f), vec4(0.0f, 1.0f, 0.0f, 1.0f), vec4(0.0f, 0.0f, 1.0f, 1.0f), vec4(1.0f, 1.0f, 1.0f, 1.0f));
  tint_symbol_1.color = colors[InstanceIndex];
  return tint_symbol_1;
}

void main() {
  Output inner_result = tint_symbol(uint(gl_VertexID), uint(gl_InstanceID));
  gl_Position = inner_result.Position;
  color_1 = inner_result.color;
  gl_Position.y = -(gl_Position.y);
  gl_Position.z = ((2.0f * gl_Position.z) - gl_Position.w);
  return;
}

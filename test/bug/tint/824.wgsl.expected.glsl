#version 310 es
precision mediump float;

struct Output {
  vec4 Position;
  vec4 color;
};
struct tint_symbol_3 {
  uint VertexIndex;
  uint InstanceIndex;
};
struct tint_symbol_4 {
  vec4 color;
  vec4 Position;
};

Output tint_symbol_inner(uint VertexIndex, uint InstanceIndex) {
  vec2 zv[4] = vec2[4](vec2(0.200000003f, 0.200000003f), vec2(0.300000012f, 0.300000012f), vec2(-0.100000001f, -0.100000001f), vec2(1.100000024f, 1.100000024f));
  float z = zv[InstanceIndex].x;
  Output tint_symbol_1 = Output(vec4(0.0f, 0.0f, 0.0f, 0.0f), vec4(0.0f, 0.0f, 0.0f, 0.0f));
  tint_symbol_1.Position = vec4(0.5f, 0.5f, z, 1.0f);
  vec4 colors[4] = vec4[4](vec4(1.0f, 0.0f, 0.0f, 1.0f), vec4(0.0f, 1.0f, 0.0f, 1.0f), vec4(0.0f, 0.0f, 1.0f, 1.0f), vec4(1.0f, 1.0f, 1.0f, 1.0f));
  tint_symbol_1.color = colors[InstanceIndex];
  return tint_symbol_1;
}

tint_symbol_4 tint_symbol(tint_symbol_3 tint_symbol_2) {
  Output inner_result = tint_symbol_inner(tint_symbol_2.VertexIndex, tint_symbol_2.InstanceIndex);
  tint_symbol_4 wrapper_result = tint_symbol_4(vec4(0.0f, 0.0f, 0.0f, 0.0f), vec4(0.0f, 0.0f, 0.0f, 0.0f));
  wrapper_result.Position = inner_result.Position;
  wrapper_result.color = inner_result.color;
  return wrapper_result;
}
out vec4 color;
void main() {
  tint_symbol_3 inputs;
  inputs.VertexIndex = uint(gl_VertexID);
  inputs.InstanceIndex = uint(gl_InstanceID);
  tint_symbol_4 outputs;
  outputs = tint_symbol(inputs);
  color = outputs.color;
  gl_Position = outputs.Position;
  gl_Position.y = -gl_Position.y;
}



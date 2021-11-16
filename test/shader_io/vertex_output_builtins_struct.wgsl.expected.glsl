#version 310 es
precision mediump float;

struct VertexOutputs {
  vec4 position;
};
struct tint_symbol_1 {
  vec4 position;
};

VertexOutputs tint_symbol_inner() {
  VertexOutputs tint_symbol_2 = VertexOutputs(vec4(1.0f, 2.0f, 3.0f, 4.0f));
  return tint_symbol_2;
}

tint_symbol_1 tint_symbol() {
  VertexOutputs inner_result = tint_symbol_inner();
  tint_symbol_1 wrapper_result = tint_symbol_1(vec4(0.0f, 0.0f, 0.0f, 0.0f));
  wrapper_result.position = inner_result.position;
  return wrapper_result;
}
void main() {
  tint_symbol_1 outputs;
  outputs = tint_symbol();
  gl_Position = outputs.position;
  gl_Position.y = -gl_Position.y;
}



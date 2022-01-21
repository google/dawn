#version 310 es
precision mediump float;

struct Input {
  vec4 position;
};
struct Output {
  vec4 position;
};
struct tint_symbol_3 {
  vec4 position;
};
struct tint_symbol_4 {
  vec4 position;
};

Output tint_symbol_inner(Input tint_symbol_1) {
  Output tint_symbol_5 = Output(tint_symbol_1.position);
  return tint_symbol_5;
}

tint_symbol_4 tint_symbol(tint_symbol_3 tint_symbol_2) {
  Input tint_symbol_6 = Input(tint_symbol_2.position);
  Output inner_result = tint_symbol_inner(tint_symbol_6);
  tint_symbol_4 wrapper_result = tint_symbol_4(vec4(0.0f, 0.0f, 0.0f, 0.0f));
  wrapper_result.position = inner_result.position;
  return wrapper_result;
}
in vec4 position;
void main() {
  tint_symbol_3 inputs;
  inputs.position = position;
  tint_symbol_4 outputs;
  outputs = tint_symbol(inputs);
  gl_Position = outputs.position;
  gl_Position.y = -gl_Position.y;
}



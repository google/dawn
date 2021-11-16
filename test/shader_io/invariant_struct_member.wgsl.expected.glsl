#version 310 es
precision mediump float;

struct Out {
  vec4 pos;
};
struct tint_symbol_1 {
  vec4 pos;
};

Out tint_symbol_inner() {
  Out tint_symbol_2 = Out(vec4(0.0f, 0.0f, 0.0f, 0.0f));
  return tint_symbol_2;
}

tint_symbol_1 tint_symbol() {
  Out inner_result = tint_symbol_inner();
  tint_symbol_1 wrapper_result = tint_symbol_1(vec4(0.0f, 0.0f, 0.0f, 0.0f));
  wrapper_result.pos = inner_result.pos;
  return wrapper_result;
}
void main() {
  tint_symbol_1 outputs;
  outputs = tint_symbol();
  gl_Position = outputs.pos;
  gl_Position.y = -gl_Position.y;
}



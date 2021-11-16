#version 310 es
precision mediump float;

vec4 tint_symbol = vec4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  tint_symbol = vec4(0.0f, 0.0f, 0.0f, 0.0f);
  return;
}

struct main_out {
  vec4 tint_symbol;
};
struct tint_symbol_2 {
  vec4 tint_symbol;
};

main_out tint_symbol_1_inner() {
  main_1();
  main_out tint_symbol_3 = main_out(tint_symbol);
  return tint_symbol_3;
}

tint_symbol_2 tint_symbol_1() {
  main_out inner_result = tint_symbol_1_inner();
  tint_symbol_2 wrapper_result = tint_symbol_2(vec4(0.0f, 0.0f, 0.0f, 0.0f));
  wrapper_result.tint_symbol = inner_result.tint_symbol;
  return wrapper_result;
}
void main() {
  tint_symbol_2 outputs;
  outputs = tint_symbol_1();
  gl_Position = outputs.tint_symbol;
  gl_Position.y = -gl_Position.y;
}



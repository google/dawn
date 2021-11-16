#version 310 es
precision mediump float;

struct Interface {
  float col1;
  float col2;
  vec4 pos;
};
struct tint_symbol {
  float col1;
  float col2;
  vec4 pos;
};

Interface vert_main_inner() {
  Interface tint_symbol_3 = Interface(0.400000006f, 0.600000024f, vec4(0.0f, 0.0f, 0.0f, 0.0f));
  return tint_symbol_3;
}

struct tint_symbol_2 {
  float col1;
  float col2;
  vec4 pos;
};

tint_symbol vert_main() {
  Interface inner_result = vert_main_inner();
  tint_symbol wrapper_result = tint_symbol(0.0f, 0.0f, vec4(0.0f, 0.0f, 0.0f, 0.0f));
  wrapper_result.col1 = inner_result.col1;
  wrapper_result.col2 = inner_result.col2;
  wrapper_result.pos = inner_result.pos;
  return wrapper_result;
}
out float col1;
out float col2;
void main() {
  tint_symbol outputs;
  outputs = vert_main();
  col1 = outputs.col1;
  col2 = outputs.col2;
  gl_Position = outputs.pos;
  gl_Position.y = -gl_Position.y;
}


#version 310 es
precision mediump float;

struct Interface {
  float col1;
  float col2;
  vec4 pos;
};
struct tint_symbol {
  float col1;
  float col2;
  vec4 pos;
};
struct tint_symbol_2 {
  float col1;
  float col2;
  vec4 pos;
};

void frag_main_inner(Interface colors) {
  float r = colors.col1;
  float g = colors.col2;
}

void frag_main(tint_symbol_2 tint_symbol_1) {
  Interface tint_symbol_3 = Interface(tint_symbol_1.col1, tint_symbol_1.col2, tint_symbol_1.pos);
  frag_main_inner(tint_symbol_3);
  return;
}
in float col1;
in float col2;
void main() {
  tint_symbol_2 inputs;
  inputs.col1 = col1;
  inputs.col2 = col2;
  inputs.pos = gl_FragCoord;
  frag_main(inputs);
}



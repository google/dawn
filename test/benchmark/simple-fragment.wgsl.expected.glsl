SKIP: FAILED

#version 310 es
precision mediump float;

struct Input {
  vec4 color;
};
struct Output {
  vec4 color;
};
struct tint_symbol_3 {
  vec4 color;
};
struct tint_symbol_4 {
  vec4 color;
};

Output tint_symbol_inner(Input tint_symbol_1) {
  Output tint_symbol_5 = Output(tint_symbol_1.color);
  return tint_symbol_5;
}

tint_symbol_4 tint_symbol(tint_symbol_3 tint_symbol_2) {
  Input tint_symbol_6 = Input(tint_symbol_2.color);
  Output inner_result = tint_symbol_inner(tint_symbol_6);
  tint_symbol_4 wrapper_result = tint_symbol_4(vec4(0.0f, 0.0f, 0.0f, 0.0f));
  wrapper_result.color = inner_result.color;
  return wrapper_result;
}
in vec4 color;
out vec4 color;
void main() {
  tint_symbol_3 inputs;
  inputs.color = color;
  tint_symbol_4 outputs;
  outputs = tint_symbol(inputs);
  color = outputs.color;
}


Error parsing GLSL shader:
ERROR: 0:30: 'color' : redefinition 
ERROR: 1 compilation errors.  No code generated.




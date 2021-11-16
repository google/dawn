#version 310 es
precision mediump float;

void bar() {
}

struct tint_symbol_1 {
  vec4 value;
};

vec4 tint_symbol_inner() {
  vec2 a = vec2(0.0f, 0.0f);
  bar();
  return vec4(0.400000006f, 0.400000006f, 0.800000012f, 1.0f);
}

tint_symbol_1 tint_symbol() {
  vec4 inner_result = tint_symbol_inner();
  tint_symbol_1 wrapper_result = tint_symbol_1(vec4(0.0f, 0.0f, 0.0f, 0.0f));
  wrapper_result.value = inner_result;
  return wrapper_result;
}
out vec4 value;
void main() {
  tint_symbol_1 outputs;
  outputs = tint_symbol();
  value = outputs.value;
}



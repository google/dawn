#version 310 es
precision mediump float;

struct MyStruct {
  float f1;
};
struct tint_symbol_1 {
  vec4 value;
};

vec4 tint_symbol_inner() {
  return vec4(0.0f, 0.0f, 0.0f, 0.0f);
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



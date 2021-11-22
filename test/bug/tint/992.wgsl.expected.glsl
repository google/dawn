#version 310 es
precision mediump float;

struct tint_symbol {
  vec4 value;
};

vec4 frag_main_inner() {
  float b = 0.0f;
  vec3 v = vec3(b);
  return vec4(v, 1.0f);
}

tint_symbol frag_main() {
  vec4 inner_result = frag_main_inner();
  tint_symbol wrapper_result = tint_symbol(vec4(0.0f, 0.0f, 0.0f, 0.0f));
  wrapper_result.value = inner_result;
  return wrapper_result;
}
out vec4 value;
void main() {
  tint_symbol outputs;
  outputs = frag_main();
  value = outputs.value;
}



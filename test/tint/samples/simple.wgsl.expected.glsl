#version 310 es
precision mediump float;

layout(location = 0) out vec4 value;
void bar() {
}

vec4 tint_symbol() {
  vec2 a = vec2(0.0f);
  bar();
  return vec4(0.400000006f, 0.400000006f, 0.800000012f, 1.0f);
}

void main() {
  vec4 inner_result = tint_symbol();
  value = inner_result;
  return;
}

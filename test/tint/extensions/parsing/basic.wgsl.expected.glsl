#version 310 es
precision mediump float;

layout(location = 0) out vec4 value;
vec4 tint_symbol() {
  return vec4(0.100000001f, 0.200000003f, 0.300000012f, 0.400000006f);
}

void main() {
  vec4 inner_result = tint_symbol();
  value = inner_result;
  return;
}

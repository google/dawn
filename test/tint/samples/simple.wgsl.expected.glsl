#version 310 es
precision highp float;
precision highp int;

layout(location = 0) out vec4 value;
void bar() {
}

vec4 tint_symbol() {
  vec2 a = vec2(0.0f);
  bar();
  return vec4(0.40000000596046447754f, 0.40000000596046447754f, 0.80000001192092895508f, 1.0f);
}

void main() {
  vec4 inner_result = tint_symbol();
  value = inner_result;
  return;
}

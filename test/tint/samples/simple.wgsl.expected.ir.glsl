#version 310 es
precision highp float;
precision highp int;

layout(location = 0) out vec4 tint_symbol_loc0_Output;
void bar() {
}
vec4 tint_symbol_inner() {
  vec2 a = vec2(0.0f);
  bar();
  return vec4(0.40000000596046447754f, 0.40000000596046447754f, 0.80000001192092895508f, 1.0f);
}
void main() {
  tint_symbol_loc0_Output = tint_symbol_inner();
}

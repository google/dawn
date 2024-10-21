#version 310 es
precision highp float;
precision highp int;

layout(location = 0) out vec4 tint_symbol_loc0_Output;
vec4 tint_symbol_inner() {
  return vec4(0.10000000149011611938f, 0.20000000298023223877f, 0.30000001192092895508f, 0.40000000596046447754f);
}
void main() {
  tint_symbol_loc0_Output = tint_symbol_inner();
}

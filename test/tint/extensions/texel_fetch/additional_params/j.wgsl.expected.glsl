#version 310 es
precision highp float;
precision highp int;

layout(location = 0) in vec4 f_loc0_Input;
layout(location = 1) flat in vec4 f_loc1_Input;
in vec4 f_Input;
void g(float a, float b, float c) {
}
void f_inner(vec4 a, vec4 b, vec4 fbf) {
  g(a.x, b.y, fbf.x);
}
void main() {
  f_inner(f_loc0_Input, f_loc1_Input, f_Input);
}

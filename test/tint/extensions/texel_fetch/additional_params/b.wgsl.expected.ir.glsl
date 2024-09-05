#version 310 es
precision highp float;
precision highp int;

layout(location = 0) in vec4 f_loc0_Input;
in vec4 f_Input;
void g(float a, float b, float c) {
}
void f_inner(vec4 pos, vec4 uv, vec4 fbf) {
  g(pos[0u], uv[0u], fbf[0u]);
}
void main() {
  f_inner(gl_FragCoord, f_loc0_Input, f_Input);
}

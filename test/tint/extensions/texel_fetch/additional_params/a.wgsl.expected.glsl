#version 310 es
precision highp float;
precision highp int;


struct In {
  vec4 uv;
};

in vec4 f_Input;
layout(location = 0) in vec4 f_loc0_Input;
void g(float a, float b, float c) {
}
void f_inner(vec4 pos, vec4 fbf, In tint_symbol) {
  g(pos[0u], fbf[0u], tint_symbol.uv[0u]);
}
void main() {
  vec4 v = gl_FragCoord;
  vec4 v_1 = f_Input;
  f_inner(v, v_1, In(f_loc0_Input));
}

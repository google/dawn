#version 310 es
precision highp float;
precision highp int;


struct In {
  vec4 pos;
};

in vec4 f_Input;
void g(float a, float b) {
}
void f_inner(In tint_symbol, vec4 fbf) {
  g(tint_symbol.pos[0u], fbf[1u]);
}
void main() {
  In v = In(gl_FragCoord);
  f_inner(v, f_Input);
}

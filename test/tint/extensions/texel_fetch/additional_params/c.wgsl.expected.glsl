#version 310 es
precision highp float;
precision highp int;


struct In {
  vec4 pos;
  vec4 uv;
  vec4 fbf;
};

layout(location = 0) in vec4 tint_interstage_location0;
in vec4 f_Input;
void g(float a, float b, float c) {
}
void f_inner(In tint_symbol) {
  g(tint_symbol.pos.x, tint_symbol.uv.x, tint_symbol.fbf.y);
}
void main() {
  f_inner(In(gl_FragCoord, tint_interstage_location0, f_Input));
}

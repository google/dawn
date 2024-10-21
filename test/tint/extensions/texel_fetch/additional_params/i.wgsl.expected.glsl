SKIP: INVALID

#version 310 es
precision highp float;
precision highp int;


struct In {
  vec4 a;
  vec4 b;
  ivec4 fbf;
};

layout(location = 0) in vec4 f_loc0_Input;
layout(location = 1) flat in vec4 f_loc1_Input;
in ivec4 f_Input;
void g(float a, float b, int c) {
}
void f_inner(In tint_symbol) {
  g(tint_symbol.a[0u], tint_symbol.b[1u], tint_symbol.fbf[0u]);
}
void main() {
  f_inner(In(f_loc0_Input, f_loc1_Input, f_Input));
}
error: Error parsing GLSL shader:
ERROR: 0:14: 'int' : must be qualified as flat in
ERROR: 0:14: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1

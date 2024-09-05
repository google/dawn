SKIP: INVALID

#version 310 es
precision highp float;
precision highp int;


struct In {
  vec4 a;
  vec4 b;
  ivec4 fbf;
};

void g(float a, float b, int c) {
}
void main(In tint_symbol) {
  g(tint_symbol.a[0u], tint_symbol.b[1u], tint_symbol.fbf[0u]);
}
error: Error parsing GLSL shader:
ERROR: 0:14: 'main' : function cannot take any parameter(s) 
ERROR: 0:14: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1

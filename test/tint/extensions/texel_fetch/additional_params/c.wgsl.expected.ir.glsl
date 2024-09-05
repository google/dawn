SKIP: INVALID

#version 310 es
precision highp float;
precision highp int;


struct In {
  vec4 pos;
  vec4 uv;
  vec4 fbf;
};

void g(float a, float b, float c) {
}
void main(In tint_symbol) {
  g(tint_symbol.pos[0u], tint_symbol.uv[0u], tint_symbol.fbf[1u]);
}
error: Error parsing GLSL shader:
ERROR: 0:14: 'main' : function cannot take any parameter(s) 
ERROR: 0:14: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1

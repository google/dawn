SKIP: INVALID

#version 310 es
precision highp float;
precision highp int;


struct In {
  ivec4 fbf;
  vec4 pos;
};

void g(int a, float b) {
}
void main(In tint_symbol) {
  g(tint_symbol.fbf[3u], tint_symbol.pos[0u]);
}
error: Error parsing GLSL shader:
ERROR: 0:13: 'main' : function cannot take any parameter(s) 
ERROR: 0:13: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1

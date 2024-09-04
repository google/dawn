SKIP: FAILED

#version 310 es
precision highp float;
precision highp int;


struct In {
  vec4 pos;
};

void g(float a, float b) {
}
void main(In tint_symbol, vec4 fbf) {
  g(tint_symbol.pos[0u], fbf[1u]);
}
error: Error parsing GLSL shader:
ERROR: 0:12: 'main' : function cannot take any parameter(s) 
ERROR: 0:12: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1

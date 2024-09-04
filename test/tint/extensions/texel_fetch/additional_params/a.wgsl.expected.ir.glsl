SKIP: FAILED

#version 310 es
precision highp float;
precision highp int;


struct In {
  vec4 uv;
};

void g(float a, float b, float c) {
}
void main(vec4 pos, vec4 fbf, In tint_symbol) {
  g(pos[0u], fbf[0u], tint_symbol.uv[0u]);
}
error: Error parsing GLSL shader:
ERROR: 0:12: 'main' : function cannot take any parameter(s) 
ERROR: 0:12: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1

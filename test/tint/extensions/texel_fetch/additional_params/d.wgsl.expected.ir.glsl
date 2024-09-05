SKIP: INVALID

#version 310 es
precision highp float;
precision highp int;


struct In {
  vec4 pos;
};

void g(int a, float b, float c, uint d) {
}
void main(ivec4 fbf_2, In tint_symbol, vec4 uv, uvec4 fbf_0) {
  g(fbf_2[2u], tint_symbol.pos[0u], uv[0u], fbf_0[1u]);
}
error: Error parsing GLSL shader:
ERROR: 0:12: 'main' : function cannot take any parameter(s) 
ERROR: 0:12: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1

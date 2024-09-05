SKIP: INVALID

#version 310 es
precision highp float;
precision highp int;


void g(float a, float b, float c) {
}
void main(vec4 pos, vec4 uv, vec4 fbf) {
  g(pos[0u], uv[0u], fbf[0u]);
}
error: Error parsing GLSL shader:
ERROR: 0:8: 'main' : function cannot take any parameter(s) 
ERROR: 0:8: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1

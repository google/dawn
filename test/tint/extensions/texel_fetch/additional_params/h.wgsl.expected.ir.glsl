SKIP: FAILED

#version 310 es
precision highp float;
precision highp int;


struct FBF {
  vec4 c1;
  ivec4 c3;
};

void g(float a, float b, int c) {
}
void main(vec4 pos, FBF fbf) {
  g(fbf.c1[0u], pos[1u], fbf.c3[2u]);
}
error: Error parsing GLSL shader:
ERROR: 0:13: 'main' : function cannot take any parameter(s) 
ERROR: 0:13: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1

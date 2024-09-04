SKIP: FAILED

#version 310 es
precision highp float;
precision highp int;


void bar() {
}
vec4 main() {
  vec2 a = vec2(0.0f);
  bar();
  return vec4(0.40000000596046447754f, 0.40000000596046447754f, 0.80000001192092895508f, 1.0f);
}
error: Error parsing GLSL shader:
ERROR: 0:8: 'float' :  entry point cannot return a value
ERROR: 0:8: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1

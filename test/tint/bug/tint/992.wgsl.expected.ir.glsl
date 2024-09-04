SKIP: FAILED

#version 310 es
precision highp float;
precision highp int;


vec4 main() {
  float b = 0.0f;
  vec3 v = vec3(b);
  return vec4(v, 1.0f);
}
error: Error parsing GLSL shader:
ERROR: 0:6: 'float' :  entry point cannot return a value
ERROR: 0:6: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1

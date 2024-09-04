SKIP: FAILED

#version 310 es
precision highp float;
precision highp int;


vec4 main() {
  return vec4(0.10000000149011611938f, 0.20000000298023223877f, 0.30000001192092895508f, 0.40000000596046447754f);
}
error: Error parsing GLSL shader:
ERROR: 0:6: 'float' :  entry point cannot return a value
ERROR: 0:6: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1

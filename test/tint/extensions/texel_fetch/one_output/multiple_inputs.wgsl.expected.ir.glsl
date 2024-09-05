SKIP: INVALID

#version 310 es
precision highp float;
precision highp int;


vec4 main(vec4 fbf_1, vec4 fbf_3) {
  return (fbf_1 + fbf_3);
}
error: Error parsing GLSL shader:
ERROR: 0:6: 'main' : function cannot take any parameter(s) 
ERROR: 0:6: 'float' :  entry point cannot return a value
ERROR: 0:6: '' : compilation terminated 
ERROR: 3 compilation errors.  No code generated.




tint executable returned error: exit status 1

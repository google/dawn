SKIP: FAILED

#version 310 es

vec4 main(uint b) {
  return vec4(float(b));
}
error: Error parsing GLSL shader:
ERROR: 0:3: 'main' : function cannot take any parameter(s) 
ERROR: 0:3: 'float' :  entry point cannot return a value
ERROR: 0:3: '' : compilation terminated 
ERROR: 3 compilation errors.  No code generated.




tint executable returned error: exit status 1

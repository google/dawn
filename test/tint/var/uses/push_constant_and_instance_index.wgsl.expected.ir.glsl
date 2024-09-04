SKIP: FAILED

#version 310 es

uniform float a;
vec4 main(uint b) {
  float v = a;
  return vec4((v + float(b)));
}
error: Error parsing GLSL shader:
ERROR: 0:4: 'main' : function cannot take any parameter(s) 
ERROR: 0:4: 'float' :  entry point cannot return a value
ERROR: 0:4: '' : compilation terminated 
ERROR: 3 compilation errors.  No code generated.




tint executable returned error: exit status 1

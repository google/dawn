SKIP: FAILED

#version 310 es

vec4 main() {
  return vec4(1.0f, 2.0f, 3.0f, 4.0f);
}
error: Error parsing GLSL shader:
ERROR: 0:3: 'float' :  entry point cannot return a value
ERROR: 0:3: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1

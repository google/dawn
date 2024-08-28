SKIP: FAILED

#version 310 es

float main() {
  return 1.0f;
}
uint main() {
  return 1u;
}
error: Error parsing GLSL shader:
ERROR: 0:3: 'float' : type requires declaration of default precision qualifier 
ERROR: 0:3: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



#version 310 es

float main() {
  return 1.0f;
}
uint main() {
  return 1u;
}
error: Error parsing GLSL shader:
ERROR: 0:3: 'float' : type requires declaration of default precision qualifier 
ERROR: 0:3: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1

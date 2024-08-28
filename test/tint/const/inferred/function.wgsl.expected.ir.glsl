SKIP: FAILED

#version 310 es

void const_decls() {
}
vec4 main() {
  return vec4(0.0f);
}
error: Error parsing GLSL shader:
ERROR: 0:5: 'float' : type requires declaration of default precision qualifier 
ERROR: 0:5: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1

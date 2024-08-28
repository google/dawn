SKIP: FAILED

#version 310 es

void main_1() {
  mat2x3 x_2 = mat2x3(vec3(50.0f, 60.0f, 50.0f), vec3(60.0f, 50.0f, 60.0f));
}
void main() {
  main_1();
}
error: Error parsing GLSL shader:
ERROR: 0:4: 'float' : type requires declaration of default precision qualifier 
ERROR: 0:4: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1

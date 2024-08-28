SKIP: FAILED

#version 310 es

void main_1() {
  mat3x2 x_2 = mat3x2(vec2(50.0f, 60.0f), vec2(60.0f, 70.0f), vec2(70.0f, 50.0f));
}
void main() {
  main_1();
}
error: Error parsing GLSL shader:
ERROR: 0:4: 'float' : type requires declaration of default precision qualifier 
ERROR: 0:4: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1

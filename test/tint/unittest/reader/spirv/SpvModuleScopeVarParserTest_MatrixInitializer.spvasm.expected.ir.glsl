SKIP: FAILED

#version 310 es

mat3x2 x_200 = mat3x2(vec2(1.5f, 2.0f), vec2(2.0f, 3.0f), vec2(3.0f, 4.0f));
void main_1() {
}
void main() {
  main_1();
}
error: Error parsing GLSL shader:
ERROR: 0:3: 'float' : type requires declaration of default precision qualifier 
ERROR: 0:3: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1

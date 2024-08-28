SKIP: FAILED

#version 310 es

void main_1() {
  vec2 x_10 = vec2(2500.0f, 3000.0f);
}
void main() {
  main_1();
}
error: Error parsing GLSL shader:
ERROR: 0:4: 'float' : type requires declaration of default precision qualifier 
ERROR: 0:4: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1

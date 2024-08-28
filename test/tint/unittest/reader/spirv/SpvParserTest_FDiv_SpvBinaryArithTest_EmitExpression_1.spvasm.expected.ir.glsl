SKIP: FAILED

#version 310 es

void main_1() {
  vec2 x_1 = vec2(0.8333333134651184082f, 1.20000004768371582031f);
}
void main() {
  main_1();
}
error: Error parsing GLSL shader:
ERROR: 0:4: 'float' : type requires declaration of default precision qualifier 
ERROR: 0:4: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1

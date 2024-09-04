SKIP: FAILED

#version 310 es

struct main_out {
  vec4 x_2_1;
};

vec4 x_2 = vec4(0.0f);
void main_1() {
}
main_out main() {
  main_1();
  return main_out(x_2);
}
error: Error parsing GLSL shader:
ERROR: 0:10: 'structure' :  entry point cannot return a value
ERROR: 0:10: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1

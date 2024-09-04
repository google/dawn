SKIP: FAILED

#version 310 es

struct main_out {
  vec4 tint_symbol;
};

vec4 tint_symbol = vec4(0.0f);
void main_1() {
  tint_symbol = vec4(0.0f);
}
main_out main() {
  main_1();
  return main_out(tint_symbol);
}
error: Error parsing GLSL shader:
ERROR: 0:11: 'structure' :  entry point cannot return a value
ERROR: 0:11: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1

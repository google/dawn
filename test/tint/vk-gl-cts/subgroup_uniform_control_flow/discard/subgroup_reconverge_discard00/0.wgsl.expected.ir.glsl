SKIP: FAILED

#version 310 es

struct main_out {
  vec4 tint_symbol;
};

vec3 pos = vec3(0.0f);
vec4 tint_symbol = vec4(0.0f);
void main_1() {
  vec3 x_21 = pos;
  tint_symbol = vec4(x_21[0u], x_21[1u], x_21[2u], 1.0f);
}
main_out main(vec3 position_param) {
  pos = position_param;
  main_1();
  return main_out(tint_symbol);
}
error: Error parsing GLSL shader:
ERROR: 0:13: 'main' : function cannot take any parameter(s) 
ERROR: 0:13: 'structure' :  entry point cannot return a value
ERROR: 0:13: '' : compilation terminated 
ERROR: 3 compilation errors.  No code generated.




tint executable returned error: exit status 1

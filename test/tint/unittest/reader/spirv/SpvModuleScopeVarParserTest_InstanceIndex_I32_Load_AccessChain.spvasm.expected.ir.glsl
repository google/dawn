SKIP: FAILED

#version 310 es


struct main_out {
  vec4 position_1_1;
};

int x_4 = 0;
vec4 position_1 = vec4(0.0f);
void main_1() {
  int x_2 = x_4;
}
main_out tint_symbol_inner(uint x_4_param) {
  x_4 = int(x_4_param);
  main_1();
  return main_out(position_1);
}
void main() {
  gl_Position = tint_symbol_inner(gl_InstanceID).position_1_1;
  gl_Position[1u] = -(gl_Position.y);
  gl_Position[2u] = ((2.0f * gl_Position.z) - gl_Position.w);
  gl_PointSize = 1.0f;
}
error: Error parsing GLSL shader:
ERROR: 0:19: 'tint_symbol_inner' : no matching overloaded function found 
ERROR: 0:19: 'assign' :  cannot convert from ' const float' to ' gl_Position highp 4-component vector of float Position'
ERROR: 0:19: '' : compilation terminated 
ERROR: 3 compilation errors.  No code generated.




tint executable returned error: exit status 1

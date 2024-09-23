SKIP: FAILED

#version 310 es


struct main_out {
  vec4 position_1_1;
};

uint x_2 = 0u;
vec4 position_1 = vec4(0.0f);
void main_1() {
}
main_out tint_symbol_inner(uint x_2_param) {
  x_2 = x_2_param;
  main_1();
  return main_out(position_1);
}
void main() {
  gl_Position = tint_symbol_inner(gl_VertexID).position_1_1;
  gl_Position[1u] = -(gl_Position.y);
  gl_Position[2u] = ((2.0f * gl_Position.z) - gl_Position.w);
  gl_PointSize = 1.0f;
}
error: Error parsing GLSL shader:
ERROR: 0:18: 'tint_symbol_inner' : no matching overloaded function found 
ERROR: 0:18: 'assign' :  cannot convert from ' const float' to ' gl_Position highp 4-component vector of float Position'
ERROR: 0:18: '' : compilation terminated 
ERROR: 3 compilation errors.  No code generated.




tint executable returned error: exit status 1

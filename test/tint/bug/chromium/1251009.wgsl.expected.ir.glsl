SKIP: FAILED

#version 310 es


struct VertexInputs0 {
  uint vertex_index;
  int loc0;
};

struct VertexInputs1 {
  uint loc1;
  vec4 loc3;
};

layout(location = 0) in int tint_symbol_loc0_Input;
layout(location = 1) in uint tint_symbol_loc1_Input;
layout(location = 2) in uint tint_symbol_loc2_Input;
layout(location = 3) in vec4 tint_symbol_loc3_Input;
vec4 tint_symbol_inner(VertexInputs0 inputs0, uint loc1, uint instance_index, VertexInputs1 inputs1) {
  uint foo = (inputs0.vertex_index + instance_index);
  return vec4(0.0f);
}
void main() {
  VertexInputs0 v = VertexInputs0(gl_VertexID, tint_symbol_loc0_Input);
  uint v_1 = tint_symbol_loc1_Input;
  uint v_2 = gl_InstanceID;
  gl_Position = tint_symbol_inner(v, v_1, v_2, VertexInputs1(tint_symbol_loc2_Input, tint_symbol_loc3_Input));
  gl_Position[1u] = -(gl_Position.y);
  gl_Position[2u] = ((2.0f * gl_Position.z) - gl_Position.w);
  gl_PointSize = 1.0f;
}
error: Error parsing GLSL shader:
ERROR: 0:23: 'constructor' :  cannot convert parameter 1 from ' gl_VertexId highp int VertexId' to ' global highp uint'
ERROR: 0:23: ' temp structure{ global highp uint vertex_index,  global highp int loc0}' : cannot construct with these arguments 
ERROR: 0:23: '=' :  cannot convert from ' const float' to ' temp structure{ global highp uint vertex_index,  global highp int loc0}'
ERROR: 0:23: '' : compilation terminated 
ERROR: 4 compilation errors.  No code generated.




tint executable returned error: exit status 1

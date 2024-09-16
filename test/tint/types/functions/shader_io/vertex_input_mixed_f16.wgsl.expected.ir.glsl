SKIP: FAILED

#version 310 es
#extension GL_AMD_gpu_shader_half_float: require


struct VertexInputs0 {
  uint vertex_index;
  int loc0;
};

struct VertexInputs1 {
  float loc2;
  vec4 loc3;
  f16vec3 loc5;
};

layout(location = 0) in int tint_symbol_loc0_Input;
layout(location = 1) in uint tint_symbol_loc1_Input;
layout(location = 2) in float tint_symbol_loc2_Input;
layout(location = 3) in vec4 tint_symbol_loc3_Input;
layout(location = 5) in f16vec3 tint_symbol_loc5_Input;
layout(location = 4) in float16_t tint_symbol_loc4_Input;
vec4 tint_symbol_inner(VertexInputs0 inputs0, uint loc1, uint instance_index, VertexInputs1 inputs1, float16_t loc4) {
  uint foo = (inputs0.vertex_index + instance_index);
  int i = inputs0.loc0;
  uint u = loc1;
  float f = inputs1.loc2;
  vec4 v = inputs1.loc3;
  float16_t x = loc4;
  f16vec3 y = inputs1.loc5;
  return vec4(0.0f);
}
void main() {
  VertexInputs0 v_1 = VertexInputs0(gl_VertexID, tint_symbol_loc0_Input);
  uint v_2 = tint_symbol_loc1_Input;
  uint v_3 = gl_InstanceID;
  VertexInputs1 v_4 = VertexInputs1(tint_symbol_loc2_Input, tint_symbol_loc3_Input, tint_symbol_loc5_Input);
  gl_Position = tint_symbol_inner(v_1, v_2, v_3, v_4, tint_symbol_loc4_Input);
  gl_Position[1u] = -(gl_Position.y);
  gl_Position[2u] = ((2.0f * gl_Position.z) - gl_Position.w);
  gl_PointSize = 1.0f;
}
error: Error parsing GLSL shader:
ERROR: 0:33: 'constructor' :  cannot convert parameter 1 from ' gl_VertexId highp int VertexId' to ' global highp uint'
ERROR: 0:33: ' temp structure{ global highp uint vertex_index,  global highp int loc0}' : cannot construct with these arguments 
ERROR: 0:33: '=' :  cannot convert from ' const float' to ' temp structure{ global highp uint vertex_index,  global highp int loc0}'
ERROR: 0:33: '' : compilation terminated 
ERROR: 4 compilation errors.  No code generated.




tint executable returned error: exit status 1

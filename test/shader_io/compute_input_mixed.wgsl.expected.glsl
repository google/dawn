#version 310 es
precision mediump float;

struct ComputeInputs0 {
  uvec3 local_invocation_id;
};
struct ComputeInputs1 {
  uvec3 workgroup_id;
};
struct tint_symbol_2 {
  uvec3 local_invocation_id;
  uint local_invocation_index;
  uvec3 global_invocation_id;
  uvec3 workgroup_id;
};

void tint_symbol_inner(ComputeInputs0 inputs0, uint local_invocation_index, uvec3 global_invocation_id, ComputeInputs1 inputs1) {
  uint foo = (((inputs0.local_invocation_id.x + local_invocation_index) + global_invocation_id.x) + inputs1.workgroup_id.x);
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void tint_symbol(tint_symbol_2 tint_symbol_1) {
  ComputeInputs0 tint_symbol_3 = ComputeInputs0(tint_symbol_1.local_invocation_id);
  ComputeInputs1 tint_symbol_4 = ComputeInputs1(tint_symbol_1.workgroup_id);
  tint_symbol_inner(tint_symbol_3, tint_symbol_1.local_invocation_index, tint_symbol_1.global_invocation_id, tint_symbol_4);
  return;
}
void main() {
  tint_symbol_2 inputs;
  inputs.local_invocation_id = gl_LocalInvocationID;
  inputs.local_invocation_index = uint(gl_LocalInvocationIndex);
  inputs.global_invocation_id = gl_GlobalInvocationID;
  inputs.workgroup_id = gl_WorkGroupID;
  tint_symbol(inputs);
}



#version 310 es


struct ComputeInputs0 {
  uvec3 local_invocation_id;
};

struct ComputeInputs1 {
  uvec3 workgroup_id;
};

void tint_symbol_inner(ComputeInputs0 inputs0, uint local_invocation_index, uvec3 global_invocation_id, ComputeInputs1 inputs1) {
  uint foo = (((inputs0.local_invocation_id[0u] + local_invocation_index) + global_invocation_id[0u]) + inputs1.workgroup_id[0u]);
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  ComputeInputs0 v = ComputeInputs0(gl_LocalInvocationID);
  uint v_1 = gl_LocalInvocationIndex;
  uvec3 v_2 = gl_GlobalInvocationID;
  tint_symbol_inner(v, v_1, v_2, ComputeInputs1(gl_WorkGroupID));
}

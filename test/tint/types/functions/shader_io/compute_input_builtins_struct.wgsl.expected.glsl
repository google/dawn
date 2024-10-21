#version 310 es


struct ComputeInputs {
  uvec3 local_invocation_id;
  uint local_invocation_index;
  uvec3 global_invocation_id;
  uvec3 workgroup_id;
  uvec3 num_workgroups;
};

void tint_symbol_inner(ComputeInputs inputs) {
  uint foo = ((((inputs.local_invocation_id[0u] + inputs.local_invocation_index) + inputs.global_invocation_id[0u]) + inputs.workgroup_id[0u]) + inputs.num_workgroups[0u]);
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  tint_symbol_inner(ComputeInputs(gl_LocalInvocationID, gl_LocalInvocationIndex, gl_GlobalInvocationID, gl_WorkGroupID, gl_NumWorkGroups));
}

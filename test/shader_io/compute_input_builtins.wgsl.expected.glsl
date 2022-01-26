#version 310 es
precision mediump float;

struct tint_symbol_2 {
  uvec3 local_invocation_id;
  uint local_invocation_index;
  uvec3 global_invocation_id;
  uvec3 workgroup_id;
  uvec3 num_workgroups;
};

void tint_symbol_inner(uvec3 local_invocation_id, uint local_invocation_index, uvec3 global_invocation_id, uvec3 workgroup_id, uvec3 num_workgroups) {
  uint foo = ((((local_invocation_id.x + local_invocation_index) + global_invocation_id.x) + workgroup_id.x) + num_workgroups.x);
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void tint_symbol(tint_symbol_2 tint_symbol_1) {
  tint_symbol_inner(tint_symbol_1.local_invocation_id, tint_symbol_1.local_invocation_index, tint_symbol_1.global_invocation_id, tint_symbol_1.workgroup_id, tint_symbol_1.num_workgroups);
  return;
}






void main() {
  tint_symbol_2 inputs;
  inputs.local_invocation_id = gl_LocalInvocationID;
  inputs.local_invocation_index = uint(gl_LocalInvocationIndex);
  inputs.global_invocation_id = gl_GlobalInvocationID;
  inputs.workgroup_id = gl_WorkGroupID;
  inputs.num_workgroups = gl_NumWorkGroups;
  tint_symbol(inputs);
}


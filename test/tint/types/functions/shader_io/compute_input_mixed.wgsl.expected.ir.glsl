SKIP: FAILED

#version 310 es

struct ComputeInputs0 {
  uvec3 local_invocation_id;
};

struct ComputeInputs1 {
  uvec3 workgroup_id;
};

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main(ComputeInputs0 inputs0, uint local_invocation_index, uvec3 global_invocation_id, ComputeInputs1 inputs1) {
  uint foo = (((inputs0.local_invocation_id[0u] + local_invocation_index) + global_invocation_id[0u]) + inputs1.workgroup_id[0u]);
}
error: Error parsing GLSL shader:
ERROR: 0:12: 'main' : function cannot take any parameter(s) 
ERROR: 0:12: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1

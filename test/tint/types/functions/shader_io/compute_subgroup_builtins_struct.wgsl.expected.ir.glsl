SKIP: INVALID

#version 310 es

struct ComputeInputs {
  uint subgroup_invocation_id;
  uint subgroup_size;
};

uint tint_symbol[];
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main(ComputeInputs inputs) {
  tint_symbol[inputs.subgroup_invocation_id] = inputs.subgroup_size;
}
error: Error parsing GLSL shader:
ERROR: 0:8: '' : array size required
ERROR: 1 compilation errors.  No code generated.




tint executable returned error: exit status 1

SKIP: INVALID

#version 310 es

uint tint_symbol[];
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main(uint subgroup_invocation_id, uint subgroup_size) {
  tint_symbol[subgroup_invocation_id] = subgroup_size;
}
error: Error parsing GLSL shader:
ERROR: 0:3: '' : array size required
ERROR: 1 compilation errors.  No code generated.




tint executable returned error: exit status 1

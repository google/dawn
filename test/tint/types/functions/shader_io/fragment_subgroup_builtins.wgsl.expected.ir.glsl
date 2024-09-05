SKIP: INVALID

#version 310 es
precision highp float;
precision highp int;


uint tint_symbol[];
void main(uint subgroup_invocation_id, uint subgroup_size) {
  tint_symbol[subgroup_invocation_id] = subgroup_size;
}
error: Error parsing GLSL shader:
ERROR: 0:6: '' : array size required
ERROR: 1 compilation errors.  No code generated.




tint executable returned error: exit status 1

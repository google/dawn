SKIP: FAILED

#version 310 es
precision highp float;
precision highp int;


struct FragmentInputs {
  uint subgroup_invocation_id;
  uint subgroup_size;
};

uint tint_symbol[];
void main(FragmentInputs inputs) {
  tint_symbol[inputs.subgroup_invocation_id] = inputs.subgroup_size;
}
error: Error parsing GLSL shader:
ERROR: 0:11: '' : array size required 
ERROR: 1 compilation errors.  No code generated.




tint executable returned error: exit status 1

SKIP: INVALID


enable chromium_experimental_subgroups;

@compute @workgroup_size(1)
fn tint_symbol() {
  let val : f32 = 2.0;
  let subadd : f32 = subgroupInclusiveAdd(val);
  let submul : f32 = subgroupInclusiveMul(val);
}

Failed to generate: <dawn>/test/tint/builtins/compute_subgroup_inclusive.wgsl:1:8 error: GLSL backend does not support extension 'chromium_experimental_subgroups'
enable chromium_experimental_subgroups;
       ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^


tint executable returned error: exit status 1

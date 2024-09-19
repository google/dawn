SKIP: INVALID


enable subgroups;
enable subgroups_f16;
enable f16;

@group(0) @binding(0) var<storage, read_write> prevent_dce : f16;

fn subgroupExclusiveAdd_4a1568() -> f16 {
  var res : f16 = subgroupExclusiveAdd(1.0h);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = subgroupExclusiveAdd_4a1568();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupExclusiveAdd_4a1568();
}

Failed to generate: <dawn>/test/tint/builtins/gen/literal/subgroupExclusiveAdd/4a1568.wgsl:41:8 error: GLSL backend does not support extension 'subgroups'
enable subgroups;
       ^^^^^^^^^


enable subgroups;
enable subgroups_f16;
enable f16;

@group(0) @binding(0) var<storage, read_write> prevent_dce : f16;

fn subgroupExclusiveAdd_4a1568() -> f16 {
  var res : f16 = subgroupExclusiveAdd(1.0h);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = subgroupExclusiveAdd_4a1568();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupExclusiveAdd_4a1568();
}

Failed to generate: <dawn>/test/tint/builtins/gen/literal/subgroupExclusiveAdd/4a1568.wgsl:41:8 error: GLSL backend does not support extension 'subgroups'
enable subgroups;
       ^^^^^^^^^


tint executable returned error: exit status 1

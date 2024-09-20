SKIP: INVALID


enable subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : u32;

fn subgroupInclusiveAdd_7ed675() -> u32 {
  var arg_0 = 1u;
  var res : u32 = subgroupInclusiveAdd(arg_0);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = subgroupInclusiveAdd_7ed675();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupInclusiveAdd_7ed675();
}

Failed to generate: <dawn>/test/tint/builtins/gen/var/subgroupInclusiveAdd/7ed675.wgsl:41:8 error: GLSL backend does not support extension 'subgroups'
enable subgroups;
       ^^^^^^^^^


enable subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : u32;

fn subgroupInclusiveAdd_7ed675() -> u32 {
  var arg_0 = 1u;
  var res : u32 = subgroupInclusiveAdd(arg_0);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = subgroupInclusiveAdd_7ed675();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupInclusiveAdd_7ed675();
}

Failed to generate: <dawn>/test/tint/builtins/gen/var/subgroupInclusiveAdd/7ed675.wgsl:41:8 error: GLSL backend does not support extension 'subgroups'
enable subgroups;
       ^^^^^^^^^


tint executable returned error: exit status 1

SKIP: INVALID


enable subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : vec2<i32>;

fn subgroupInclusiveAdd_fabbde() -> vec2<i32> {
  var arg_0 = vec2<i32>(1i);
  var res : vec2<i32> = subgroupInclusiveAdd(arg_0);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = subgroupInclusiveAdd_fabbde();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupInclusiveAdd_fabbde();
}

Failed to generate: <dawn>/test/tint/builtins/gen/var/subgroupInclusiveAdd/fabbde.wgsl:41:8 error: GLSL backend does not support extension 'subgroups'
enable subgroups;
       ^^^^^^^^^


enable subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : vec2<i32>;

fn subgroupInclusiveAdd_fabbde() -> vec2<i32> {
  var arg_0 = vec2<i32>(1i);
  var res : vec2<i32> = subgroupInclusiveAdd(arg_0);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = subgroupInclusiveAdd_fabbde();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupInclusiveAdd_fabbde();
}

Failed to generate: <dawn>/test/tint/builtins/gen/var/subgroupInclusiveAdd/fabbde.wgsl:41:8 error: GLSL backend does not support extension 'subgroups'
enable subgroups;
       ^^^^^^^^^


tint executable returned error: exit status 1

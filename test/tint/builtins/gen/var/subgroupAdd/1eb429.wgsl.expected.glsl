SKIP: INVALID


enable subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : vec2<i32>;

fn subgroupAdd_1eb429() -> vec2<i32> {
  var arg_0 = vec2<i32>(1i);
  var res : vec2<i32> = subgroupAdd(arg_0);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = subgroupAdd_1eb429();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupAdd_1eb429();
}

Failed to generate: <dawn>/test/tint/builtins/gen/var/subgroupAdd/1eb429.wgsl:41:8 error: GLSL backend does not support extension 'subgroups'
enable subgroups;
       ^^^^^^^^^


enable subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : vec2<i32>;

fn subgroupAdd_1eb429() -> vec2<i32> {
  var arg_0 = vec2<i32>(1i);
  var res : vec2<i32> = subgroupAdd(arg_0);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = subgroupAdd_1eb429();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupAdd_1eb429();
}

Failed to generate: <dawn>/test/tint/builtins/gen/var/subgroupAdd/1eb429.wgsl:41:8 error: GLSL backend does not support extension 'subgroups'
enable subgroups;
       ^^^^^^^^^


tint executable returned error: exit status 1

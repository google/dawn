SKIP: INVALID


enable subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : vec2<u32>;

fn subgroupInclusiveAdd_9bbcb0() -> vec2<u32> {
  var arg_0 = vec2<u32>(1u);
  var res : vec2<u32> = subgroupInclusiveAdd(arg_0);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = subgroupInclusiveAdd_9bbcb0();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupInclusiveAdd_9bbcb0();
}

Failed to generate: <dawn>/test/tint/builtins/gen/var/subgroupInclusiveAdd/9bbcb0.wgsl:41:8 error: GLSL backend does not support extension 'subgroups'
enable subgroups;
       ^^^^^^^^^


enable subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : vec2<u32>;

fn subgroupInclusiveAdd_9bbcb0() -> vec2<u32> {
  var arg_0 = vec2<u32>(1u);
  var res : vec2<u32> = subgroupInclusiveAdd(arg_0);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = subgroupInclusiveAdd_9bbcb0();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupInclusiveAdd_9bbcb0();
}

Failed to generate: <dawn>/test/tint/builtins/gen/var/subgroupInclusiveAdd/9bbcb0.wgsl:41:8 error: GLSL backend does not support extension 'subgroups'
enable subgroups;
       ^^^^^^^^^


tint executable returned error: exit status 1

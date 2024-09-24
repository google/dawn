SKIP: INVALID


enable subgroups;
enable subgroups_f16;
enable f16;

@group(0) @binding(0) var<storage, read_write> prevent_dce : vec2<f16>;

fn subgroupInclusiveAdd_a7c60f() -> vec2<f16> {
  var res : vec2<f16> = subgroupInclusiveAdd(vec2<f16>(1.0h));
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = subgroupInclusiveAdd_a7c60f();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupInclusiveAdd_a7c60f();
}

Failed to generate: <dawn>/test/tint/builtins/gen/literal/subgroupInclusiveAdd/a7c60f.wgsl:41:8 error: GLSL backend does not support extension 'subgroups'
enable subgroups;
       ^^^^^^^^^


enable subgroups;
enable subgroups_f16;
enable f16;

@group(0) @binding(0) var<storage, read_write> prevent_dce : vec2<f16>;

fn subgroupInclusiveAdd_a7c60f() -> vec2<f16> {
  var res : vec2<f16> = subgroupInclusiveAdd(vec2<f16>(1.0h));
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = subgroupInclusiveAdd_a7c60f();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupInclusiveAdd_a7c60f();
}

Failed to generate: <dawn>/test/tint/builtins/gen/literal/subgroupInclusiveAdd/a7c60f.wgsl:41:8 error: GLSL backend does not support extension 'subgroups'
enable subgroups;
       ^^^^^^^^^


tint executable returned error: exit status 1

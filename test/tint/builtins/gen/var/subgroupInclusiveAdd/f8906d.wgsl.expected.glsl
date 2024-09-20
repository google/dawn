SKIP: INVALID


enable subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : vec2<f32>;

fn subgroupInclusiveAdd_f8906d() -> vec2<f32> {
  var arg_0 = vec2<f32>(1.0f);
  var res : vec2<f32> = subgroupInclusiveAdd(arg_0);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = subgroupInclusiveAdd_f8906d();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupInclusiveAdd_f8906d();
}

Failed to generate: <dawn>/test/tint/builtins/gen/var/subgroupInclusiveAdd/f8906d.wgsl:41:8 error: GLSL backend does not support extension 'subgroups'
enable subgroups;
       ^^^^^^^^^


enable subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : vec2<f32>;

fn subgroupInclusiveAdd_f8906d() -> vec2<f32> {
  var arg_0 = vec2<f32>(1.0f);
  var res : vec2<f32> = subgroupInclusiveAdd(arg_0);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = subgroupInclusiveAdd_f8906d();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupInclusiveAdd_f8906d();
}

Failed to generate: <dawn>/test/tint/builtins/gen/var/subgroupInclusiveAdd/f8906d.wgsl:41:8 error: GLSL backend does not support extension 'subgroups'
enable subgroups;
       ^^^^^^^^^


tint executable returned error: exit status 1

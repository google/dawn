SKIP: INVALID


enable subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : vec2<f32>;

fn subgroupAdd_dcf73f() -> vec2<f32> {
  var res : vec2<f32> = subgroupAdd(vec2<f32>(1.0f));
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = subgroupAdd_dcf73f();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupAdd_dcf73f();
}

Failed to generate: <dawn>/test/tint/builtins/gen/literal/subgroupAdd/dcf73f.wgsl:41:8 error: GLSL backend does not support extension 'subgroups'
enable subgroups;
       ^^^^^^^^^


enable subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : vec2<f32>;

fn subgroupAdd_dcf73f() -> vec2<f32> {
  var res : vec2<f32> = subgroupAdd(vec2<f32>(1.0f));
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = subgroupAdd_dcf73f();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupAdd_dcf73f();
}

Failed to generate: <dawn>/test/tint/builtins/gen/literal/subgroupAdd/dcf73f.wgsl:41:8 error: GLSL backend does not support extension 'subgroups'
enable subgroups;
       ^^^^^^^^^


tint executable returned error: exit status 1

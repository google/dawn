SKIP: INVALID


enable subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : vec4<i32>;

fn subgroupOr_4d4eb0() -> vec4<i32> {
  var res : vec4<i32> = subgroupOr(vec4<i32>(1i));
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = subgroupOr_4d4eb0();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupOr_4d4eb0();
}

Failed to generate: <dawn>/test/tint/builtins/gen/literal/subgroupOr/4d4eb0.wgsl:41:8 error: GLSL backend does not support extension 'subgroups'
enable subgroups;
       ^^^^^^^^^


enable subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : vec4<i32>;

fn subgroupOr_4d4eb0() -> vec4<i32> {
  var res : vec4<i32> = subgroupOr(vec4<i32>(1i));
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = subgroupOr_4d4eb0();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupOr_4d4eb0();
}

Failed to generate: <dawn>/test/tint/builtins/gen/literal/subgroupOr/4d4eb0.wgsl:41:8 error: GLSL backend does not support extension 'subgroups'
enable subgroups;
       ^^^^^^^^^


tint executable returned error: exit status 1

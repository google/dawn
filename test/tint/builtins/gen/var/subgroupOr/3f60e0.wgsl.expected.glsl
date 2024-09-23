SKIP: INVALID


enable subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : vec2<i32>;

fn subgroupOr_3f60e0() -> vec2<i32> {
  var arg_0 = vec2<i32>(1i);
  var res : vec2<i32> = subgroupOr(arg_0);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = subgroupOr_3f60e0();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupOr_3f60e0();
}

Failed to generate: <dawn>/test/tint/builtins/gen/var/subgroupOr/3f60e0.wgsl:41:8 error: GLSL backend does not support extension 'subgroups'
enable subgroups;
       ^^^^^^^^^


enable subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : vec2<i32>;

fn subgroupOr_3f60e0() -> vec2<i32> {
  var arg_0 = vec2<i32>(1i);
  var res : vec2<i32> = subgroupOr(arg_0);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = subgroupOr_3f60e0();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupOr_3f60e0();
}

Failed to generate: <dawn>/test/tint/builtins/gen/var/subgroupOr/3f60e0.wgsl:41:8 error: GLSL backend does not support extension 'subgroups'
enable subgroups;
       ^^^^^^^^^


tint executable returned error: exit status 1

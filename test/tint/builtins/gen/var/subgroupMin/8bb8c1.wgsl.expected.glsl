SKIP: INVALID


enable subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : vec2<u32>;

fn subgroupMin_8bb8c1() -> vec2<u32> {
  var arg_0 = vec2<u32>(1u);
  var res : vec2<u32> = subgroupMin(arg_0);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = subgroupMin_8bb8c1();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupMin_8bb8c1();
}

Failed to generate: <dawn>/test/tint/builtins/gen/var/subgroupMin/8bb8c1.wgsl:41:8 error: GLSL backend does not support extension 'subgroups'
enable subgroups;
       ^^^^^^^^^


enable subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : vec2<u32>;

fn subgroupMin_8bb8c1() -> vec2<u32> {
  var arg_0 = vec2<u32>(1u);
  var res : vec2<u32> = subgroupMin(arg_0);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = subgroupMin_8bb8c1();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupMin_8bb8c1();
}

Failed to generate: <dawn>/test/tint/builtins/gen/var/subgroupMin/8bb8c1.wgsl:41:8 error: GLSL backend does not support extension 'subgroups'
enable subgroups;
       ^^^^^^^^^


tint executable returned error: exit status 1

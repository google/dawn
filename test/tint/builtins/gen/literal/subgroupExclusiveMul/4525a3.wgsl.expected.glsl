SKIP: INVALID


enable subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : vec2<i32>;

fn subgroupExclusiveMul_4525a3() -> vec2<i32> {
  var res : vec2<i32> = subgroupExclusiveMul(vec2<i32>(1i));
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = subgroupExclusiveMul_4525a3();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupExclusiveMul_4525a3();
}

Failed to generate: <dawn>/test/tint/builtins/gen/literal/subgroupExclusiveMul/4525a3.wgsl:41:8 error: GLSL backend does not support extension 'subgroups'
enable subgroups;
       ^^^^^^^^^


enable subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : vec2<i32>;

fn subgroupExclusiveMul_4525a3() -> vec2<i32> {
  var res : vec2<i32> = subgroupExclusiveMul(vec2<i32>(1i));
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = subgroupExclusiveMul_4525a3();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupExclusiveMul_4525a3();
}

Failed to generate: <dawn>/test/tint/builtins/gen/literal/subgroupExclusiveMul/4525a3.wgsl:41:8 error: GLSL backend does not support extension 'subgroups'
enable subgroups;
       ^^^^^^^^^


tint executable returned error: exit status 1

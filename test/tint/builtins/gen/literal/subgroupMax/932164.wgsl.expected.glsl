SKIP: FAILED


enable subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : vec2<i32>;

fn subgroupMax_932164() -> vec2<i32> {
  var res : vec2<i32> = subgroupMax(vec2<i32>(1i));
  return res;
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupMax_932164();
}

Failed to generate: <dawn>/test/tint/builtins/gen/literal/subgroupMax/932164.wgsl:41:8 error: GLSL backend does not support extension 'subgroups'
enable subgroups;
       ^^^^^^^^^


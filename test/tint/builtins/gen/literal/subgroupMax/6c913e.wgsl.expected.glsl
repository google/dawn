SKIP: INVALID


enable subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : i32;

fn subgroupMax_6c913e() -> i32 {
  var res : i32 = subgroupMax(1i);
  return res;
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupMax_6c913e();
}

Failed to generate: <dawn>/test/tint/builtins/gen/literal/subgroupMax/6c913e.wgsl:41:8 error: GLSL backend does not support extension 'subgroups'
enable subgroups;
       ^^^^^^^^^


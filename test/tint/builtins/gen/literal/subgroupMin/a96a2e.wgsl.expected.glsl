SKIP: INVALID


enable subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : i32;

fn subgroupMin_a96a2e() -> i32 {
  var res : i32 = subgroupMin(1i);
  return res;
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupMin_a96a2e();
}

Failed to generate: <dawn>/test/tint/builtins/gen/literal/subgroupMin/a96a2e.wgsl:41:8 error: GLSL backend does not support extension 'subgroups'
enable subgroups;
       ^^^^^^^^^


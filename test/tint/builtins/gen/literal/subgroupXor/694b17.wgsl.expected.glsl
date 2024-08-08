SKIP: FAILED


enable subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : i32;

fn subgroupXor_694b17() -> i32 {
  var res : i32 = subgroupXor(1i);
  return res;
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupXor_694b17();
}

Failed to generate: <dawn>/test/tint/builtins/gen/literal/subgroupXor/694b17.wgsl:41:8 error: GLSL backend does not support extension 'subgroups'
enable subgroups;
       ^^^^^^^^^


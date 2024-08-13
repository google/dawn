SKIP: FAILED


enable subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : vec4<u32>;

fn subgroupShuffleXor_08f588() -> vec4<u32> {
  var res : vec4<u32> = subgroupShuffleXor(vec4<u32>(1u), 1u);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = subgroupShuffleXor_08f588();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupShuffleXor_08f588();
}

Failed to generate: <dawn>/test/tint/builtins/gen/literal/subgroupShuffleXor/08f588.wgsl:41:8 error: GLSL backend does not support extension 'subgroups'
enable subgroups;
       ^^^^^^^^^


enable subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : vec4<u32>;

fn subgroupShuffleXor_08f588() -> vec4<u32> {
  var res : vec4<u32> = subgroupShuffleXor(vec4<u32>(1u), 1u);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = subgroupShuffleXor_08f588();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupShuffleXor_08f588();
}

Failed to generate: <dawn>/test/tint/builtins/gen/literal/subgroupShuffleXor/08f588.wgsl:41:8 error: GLSL backend does not support extension 'subgroups'
enable subgroups;
       ^^^^^^^^^


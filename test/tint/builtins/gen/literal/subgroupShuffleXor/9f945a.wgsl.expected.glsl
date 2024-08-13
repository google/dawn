SKIP: FAILED


enable subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : vec3<u32>;

fn subgroupShuffleXor_9f945a() -> vec3<u32> {
  var res : vec3<u32> = subgroupShuffleXor(vec3<u32>(1u), 1u);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = subgroupShuffleXor_9f945a();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupShuffleXor_9f945a();
}

Failed to generate: <dawn>/test/tint/builtins/gen/literal/subgroupShuffleXor/9f945a.wgsl:41:8 error: GLSL backend does not support extension 'subgroups'
enable subgroups;
       ^^^^^^^^^


enable subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : vec3<u32>;

fn subgroupShuffleXor_9f945a() -> vec3<u32> {
  var res : vec3<u32> = subgroupShuffleXor(vec3<u32>(1u), 1u);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = subgroupShuffleXor_9f945a();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupShuffleXor_9f945a();
}

Failed to generate: <dawn>/test/tint/builtins/gen/literal/subgroupShuffleXor/9f945a.wgsl:41:8 error: GLSL backend does not support extension 'subgroups'
enable subgroups;
       ^^^^^^^^^


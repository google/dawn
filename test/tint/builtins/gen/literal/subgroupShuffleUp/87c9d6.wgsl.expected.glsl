SKIP: FAILED


enable subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : vec3<f32>;

fn subgroupShuffleUp_87c9d6() -> vec3<f32> {
  var res : vec3<f32> = subgroupShuffleUp(vec3<f32>(1.0f), 1u);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = subgroupShuffleUp_87c9d6();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupShuffleUp_87c9d6();
}

Failed to generate: <dawn>/test/tint/builtins/gen/literal/subgroupShuffleUp/87c9d6.wgsl:41:8 error: GLSL backend does not support extension 'subgroups'
enable subgroups;
       ^^^^^^^^^


enable subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : vec3<f32>;

fn subgroupShuffleUp_87c9d6() -> vec3<f32> {
  var res : vec3<f32> = subgroupShuffleUp(vec3<f32>(1.0f), 1u);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = subgroupShuffleUp_87c9d6();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupShuffleUp_87c9d6();
}

Failed to generate: <dawn>/test/tint/builtins/gen/literal/subgroupShuffleUp/87c9d6.wgsl:41:8 error: GLSL backend does not support extension 'subgroups'
enable subgroups;
       ^^^^^^^^^


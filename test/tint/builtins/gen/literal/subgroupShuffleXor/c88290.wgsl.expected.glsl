SKIP: FAILED


enable subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : vec4<f32>;

fn subgroupShuffleXor_c88290() -> vec4<f32> {
  var res : vec4<f32> = subgroupShuffleXor(vec4<f32>(1.0f), 1u);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = subgroupShuffleXor_c88290();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupShuffleXor_c88290();
}

Failed to generate: <dawn>/test/tint/builtins/gen/literal/subgroupShuffleXor/c88290.wgsl:41:8 error: GLSL backend does not support extension 'subgroups'
enable subgroups;
       ^^^^^^^^^


enable subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : vec4<f32>;

fn subgroupShuffleXor_c88290() -> vec4<f32> {
  var res : vec4<f32> = subgroupShuffleXor(vec4<f32>(1.0f), 1u);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = subgroupShuffleXor_c88290();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupShuffleXor_c88290();
}

Failed to generate: <dawn>/test/tint/builtins/gen/literal/subgroupShuffleXor/c88290.wgsl:41:8 error: GLSL backend does not support extension 'subgroups'
enable subgroups;
       ^^^^^^^^^


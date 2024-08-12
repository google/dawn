SKIP: FAILED


enable subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : vec2<f32>;

fn subgroupShuffle_d9ff67() -> vec2<f32> {
  var res : vec2<f32> = subgroupShuffle(vec2<f32>(1.0f), 1i);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = subgroupShuffle_d9ff67();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupShuffle_d9ff67();
}

Failed to generate: <dawn>/test/tint/builtins/gen/literal/subgroupShuffle/d9ff67.wgsl:41:8 error: GLSL backend does not support extension 'subgroups'
enable subgroups;
       ^^^^^^^^^


enable subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : vec2<f32>;

fn subgroupShuffle_d9ff67() -> vec2<f32> {
  var res : vec2<f32> = subgroupShuffle(vec2<f32>(1.0f), 1i);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = subgroupShuffle_d9ff67();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupShuffle_d9ff67();
}

Failed to generate: <dawn>/test/tint/builtins/gen/literal/subgroupShuffle/d9ff67.wgsl:41:8 error: GLSL backend does not support extension 'subgroups'
enable subgroups;
       ^^^^^^^^^


SKIP: INVALID


enable subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : vec2<f32>;

fn subgroupMax_1fc846() -> vec2<f32> {
  var res : vec2<f32> = subgroupMax(vec2<f32>(1.0f));
  return res;
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupMax_1fc846();
}

Failed to generate: <dawn>/test/tint/builtins/gen/literal/subgroupMax/1fc846.wgsl:41:8 error: GLSL backend does not support extension 'subgroups'
enable subgroups;
       ^^^^^^^^^


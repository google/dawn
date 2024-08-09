SKIP: FAILED


enable subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : vec4<f32>;

fn subgroupMin_bbd9b0() -> vec4<f32> {
  var res : vec4<f32> = subgroupMin(vec4<f32>(1.0f));
  return res;
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupMin_bbd9b0();
}

Failed to generate: <dawn>/test/tint/builtins/gen/literal/subgroupMin/bbd9b0.wgsl:41:8 error: GLSL backend does not support extension 'subgroups'
enable subgroups;
       ^^^^^^^^^


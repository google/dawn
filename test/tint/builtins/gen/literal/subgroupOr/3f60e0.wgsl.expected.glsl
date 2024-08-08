SKIP: FAILED


enable subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : vec2<i32>;

fn subgroupOr_3f60e0() -> vec2<i32> {
  var res : vec2<i32> = subgroupOr(vec2<i32>(1i));
  return res;
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupOr_3f60e0();
}

Failed to generate: <dawn>/test/tint/builtins/gen/literal/subgroupOr/3f60e0.wgsl:41:8 error: GLSL backend does not support extension 'subgroups'
enable subgroups;
       ^^^^^^^^^


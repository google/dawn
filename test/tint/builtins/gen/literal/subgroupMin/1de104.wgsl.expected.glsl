SKIP: INVALID


enable subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : vec4<i32>;

fn subgroupMin_1de104() -> vec4<i32> {
  var res : vec4<i32> = subgroupMin(vec4<i32>(1i));
  return res;
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupMin_1de104();
}

Failed to generate: <dawn>/test/tint/builtins/gen/literal/subgroupMin/1de104.wgsl:41:8 error: GLSL backend does not support extension 'subgroups'
enable subgroups;
       ^^^^^^^^^


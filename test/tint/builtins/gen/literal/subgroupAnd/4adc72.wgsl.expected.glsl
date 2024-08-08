SKIP: FAILED


enable subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : vec2<i32>;

fn subgroupAnd_4adc72() -> vec2<i32> {
  var res : vec2<i32> = subgroupAnd(vec2<i32>(1i));
  return res;
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupAnd_4adc72();
}

Failed to generate: <dawn>/test/tint/builtins/gen/literal/subgroupAnd/4adc72.wgsl:41:8 error: GLSL backend does not support extension 'subgroups'
enable subgroups;
       ^^^^^^^^^


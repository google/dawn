SKIP: INVALID


enable subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : vec4<u32>;

fn subgroupXor_9d77e4() -> vec4<u32> {
  var res : vec4<u32> = subgroupXor(vec4<u32>(1u));
  return res;
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupXor_9d77e4();
}

Failed to generate: <dawn>/test/tint/builtins/gen/literal/subgroupXor/9d77e4.wgsl:41:8 error: GLSL backend does not support extension 'subgroups'
enable subgroups;
       ^^^^^^^^^


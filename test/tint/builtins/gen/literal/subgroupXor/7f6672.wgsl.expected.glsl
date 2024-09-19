SKIP: INVALID


enable subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : vec2<u32>;

fn subgroupXor_7f6672() -> vec2<u32> {
  var res : vec2<u32> = subgroupXor(vec2<u32>(1u));
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = subgroupXor_7f6672();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupXor_7f6672();
}

Failed to generate: <dawn>/test/tint/builtins/gen/literal/subgroupXor/7f6672.wgsl:41:8 error: GLSL backend does not support extension 'subgroups'
enable subgroups;
       ^^^^^^^^^


enable subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : vec2<u32>;

fn subgroupXor_7f6672() -> vec2<u32> {
  var res : vec2<u32> = subgroupXor(vec2<u32>(1u));
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = subgroupXor_7f6672();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupXor_7f6672();
}

Failed to generate: <dawn>/test/tint/builtins/gen/literal/subgroupXor/7f6672.wgsl:41:8 error: GLSL backend does not support extension 'subgroups'
enable subgroups;
       ^^^^^^^^^


tint executable returned error: exit status 1

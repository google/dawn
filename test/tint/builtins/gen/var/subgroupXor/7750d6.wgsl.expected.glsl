SKIP: INVALID


enable subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : u32;

fn subgroupXor_7750d6() -> u32 {
  var arg_0 = 1u;
  var res : u32 = subgroupXor(arg_0);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = subgroupXor_7750d6();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupXor_7750d6();
}

Failed to generate: <dawn>/test/tint/builtins/gen/var/subgroupXor/7750d6.wgsl:41:8 error: GLSL backend does not support extension 'subgroups'
enable subgroups;
       ^^^^^^^^^


enable subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : u32;

fn subgroupXor_7750d6() -> u32 {
  var arg_0 = 1u;
  var res : u32 = subgroupXor(arg_0);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = subgroupXor_7750d6();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupXor_7750d6();
}

Failed to generate: <dawn>/test/tint/builtins/gen/var/subgroupXor/7750d6.wgsl:41:8 error: GLSL backend does not support extension 'subgroups'
enable subgroups;
       ^^^^^^^^^


tint executable returned error: exit status 1

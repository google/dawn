SKIP: INVALID


enable subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : vec4<u32>;

fn subgroupMul_dd1333() -> vec4<u32> {
  var arg_0 = vec4<u32>(1u);
  var res : vec4<u32> = subgroupMul(arg_0);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = subgroupMul_dd1333();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupMul_dd1333();
}

Failed to generate: <dawn>/test/tint/builtins/gen/var/subgroupMul/dd1333.wgsl:41:8 error: GLSL backend does not support extension 'subgroups'
enable subgroups;
       ^^^^^^^^^


enable subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : vec4<u32>;

fn subgroupMul_dd1333() -> vec4<u32> {
  var arg_0 = vec4<u32>(1u);
  var res : vec4<u32> = subgroupMul(arg_0);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = subgroupMul_dd1333();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupMul_dd1333();
}

Failed to generate: <dawn>/test/tint/builtins/gen/var/subgroupMul/dd1333.wgsl:41:8 error: GLSL backend does not support extension 'subgroups'
enable subgroups;
       ^^^^^^^^^


tint executable returned error: exit status 1

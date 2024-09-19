SKIP: INVALID


enable subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : vec4<i32>;

fn subgroupXor_83b1f3() -> vec4<i32> {
  var arg_0 = vec4<i32>(1i);
  var res : vec4<i32> = subgroupXor(arg_0);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = subgroupXor_83b1f3();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupXor_83b1f3();
}

Failed to generate: <dawn>/test/tint/builtins/gen/var/subgroupXor/83b1f3.wgsl:41:8 error: GLSL backend does not support extension 'subgroups'
enable subgroups;
       ^^^^^^^^^


enable subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : vec4<i32>;

fn subgroupXor_83b1f3() -> vec4<i32> {
  var arg_0 = vec4<i32>(1i);
  var res : vec4<i32> = subgroupXor(arg_0);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = subgroupXor_83b1f3();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupXor_83b1f3();
}

Failed to generate: <dawn>/test/tint/builtins/gen/var/subgroupXor/83b1f3.wgsl:41:8 error: GLSL backend does not support extension 'subgroups'
enable subgroups;
       ^^^^^^^^^


tint executable returned error: exit status 1

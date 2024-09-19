SKIP: INVALID


enable subgroups;
enable subgroups_f16;
enable f16;

@group(0) @binding(0) var<storage, read_write> prevent_dce : vec4<f16>;

fn subgroupBroadcast_0f44e2() -> vec4<f16> {
  var arg_0 = vec4<f16>(1.0h);
  const arg_1 = 1u;
  var res : vec4<f16> = subgroupBroadcast(arg_0, arg_1);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = subgroupBroadcast_0f44e2();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupBroadcast_0f44e2();
}

Failed to generate: <dawn>/test/tint/builtins/gen/var/subgroupBroadcast/0f44e2.wgsl:41:8 error: GLSL backend does not support extension 'subgroups'
enable subgroups;
       ^^^^^^^^^


enable subgroups;
enable subgroups_f16;
enable f16;

@group(0) @binding(0) var<storage, read_write> prevent_dce : vec4<f16>;

fn subgroupBroadcast_0f44e2() -> vec4<f16> {
  var arg_0 = vec4<f16>(1.0h);
  const arg_1 = 1u;
  var res : vec4<f16> = subgroupBroadcast(arg_0, arg_1);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = subgroupBroadcast_0f44e2();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupBroadcast_0f44e2();
}

Failed to generate: <dawn>/test/tint/builtins/gen/var/subgroupBroadcast/0f44e2.wgsl:41:8 error: GLSL backend does not support extension 'subgroups'
enable subgroups;
       ^^^^^^^^^


tint executable returned error: exit status 1

SKIP: FAILED


enable subgroups;
enable subgroups_f16;
enable f16;

@group(0) @binding(0) var<storage, read_write> prevent_dce : vec3<f16>;

fn subgroupShuffleDown_63fdb0() -> vec3<f16> {
  var arg_0 = vec3<f16>(1.0h);
  var arg_1 = 1u;
  var res : vec3<f16> = subgroupShuffleDown(arg_0, arg_1);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = subgroupShuffleDown_63fdb0();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupShuffleDown_63fdb0();
}

Failed to generate: <dawn>/test/tint/builtins/gen/var/subgroupShuffleDown/63fdb0.wgsl:41:8 error: GLSL backend does not support extension 'subgroups'
enable subgroups;
       ^^^^^^^^^


enable subgroups;
enable subgroups_f16;
enable f16;

@group(0) @binding(0) var<storage, read_write> prevent_dce : vec3<f16>;

fn subgroupShuffleDown_63fdb0() -> vec3<f16> {
  var arg_0 = vec3<f16>(1.0h);
  var arg_1 = 1u;
  var res : vec3<f16> = subgroupShuffleDown(arg_0, arg_1);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = subgroupShuffleDown_63fdb0();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupShuffleDown_63fdb0();
}

Failed to generate: <dawn>/test/tint/builtins/gen/var/subgroupShuffleDown/63fdb0.wgsl:41:8 error: GLSL backend does not support extension 'subgroups'
enable subgroups;
       ^^^^^^^^^


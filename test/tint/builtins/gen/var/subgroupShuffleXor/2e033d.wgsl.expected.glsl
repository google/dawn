SKIP: FAILED


enable subgroups;
enable subgroups_f16;
enable f16;

@group(0) @binding(0) var<storage, read_write> prevent_dce : vec4<f16>;

fn subgroupShuffleXor_2e033d() -> vec4<f16> {
  var arg_0 = vec4<f16>(1.0h);
  var arg_1 = 1u;
  var res : vec4<f16> = subgroupShuffleXor(arg_0, arg_1);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = subgroupShuffleXor_2e033d();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupShuffleXor_2e033d();
}

Failed to generate: <dawn>/test/tint/builtins/gen/var/subgroupShuffleXor/2e033d.wgsl:41:8 error: GLSL backend does not support extension 'subgroups'
enable subgroups;
       ^^^^^^^^^


enable subgroups;
enable subgroups_f16;
enable f16;

@group(0) @binding(0) var<storage, read_write> prevent_dce : vec4<f16>;

fn subgroupShuffleXor_2e033d() -> vec4<f16> {
  var arg_0 = vec4<f16>(1.0h);
  var arg_1 = 1u;
  var res : vec4<f16> = subgroupShuffleXor(arg_0, arg_1);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = subgroupShuffleXor_2e033d();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupShuffleXor_2e033d();
}

Failed to generate: <dawn>/test/tint/builtins/gen/var/subgroupShuffleXor/2e033d.wgsl:41:8 error: GLSL backend does not support extension 'subgroups'
enable subgroups;
       ^^^^^^^^^


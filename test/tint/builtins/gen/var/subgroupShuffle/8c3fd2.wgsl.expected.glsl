SKIP: FAILED


enable subgroups;
enable subgroups_f16;
enable f16;

@group(0) @binding(0) var<storage, read_write> prevent_dce : vec2<f16>;

fn subgroupShuffle_8c3fd2() -> vec2<f16> {
  var arg_0 = vec2<f16>(1.0h);
  var arg_1 = 1i;
  var res : vec2<f16> = subgroupShuffle(arg_0, arg_1);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = subgroupShuffle_8c3fd2();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupShuffle_8c3fd2();
}

Failed to generate: <dawn>/test/tint/builtins/gen/var/subgroupShuffle/8c3fd2.wgsl:41:8 error: GLSL backend does not support extension 'subgroups'
enable subgroups;
       ^^^^^^^^^


enable subgroups;
enable subgroups_f16;
enable f16;

@group(0) @binding(0) var<storage, read_write> prevent_dce : vec2<f16>;

fn subgroupShuffle_8c3fd2() -> vec2<f16> {
  var arg_0 = vec2<f16>(1.0h);
  var arg_1 = 1i;
  var res : vec2<f16> = subgroupShuffle(arg_0, arg_1);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = subgroupShuffle_8c3fd2();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupShuffle_8c3fd2();
}

Failed to generate: <dawn>/test/tint/builtins/gen/var/subgroupShuffle/8c3fd2.wgsl:41:8 error: GLSL backend does not support extension 'subgroups'
enable subgroups;
       ^^^^^^^^^


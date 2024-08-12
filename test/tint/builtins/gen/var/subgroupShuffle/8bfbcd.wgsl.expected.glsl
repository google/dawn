SKIP: FAILED


enable subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : i32;

fn subgroupShuffle_8bfbcd() -> i32 {
  var arg_0 = 1i;
  var arg_1 = 1i;
  var res : i32 = subgroupShuffle(arg_0, arg_1);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = subgroupShuffle_8bfbcd();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupShuffle_8bfbcd();
}

Failed to generate: <dawn>/test/tint/builtins/gen/var/subgroupShuffle/8bfbcd.wgsl:41:8 error: GLSL backend does not support extension 'subgroups'
enable subgroups;
       ^^^^^^^^^


enable subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : i32;

fn subgroupShuffle_8bfbcd() -> i32 {
  var arg_0 = 1i;
  var arg_1 = 1i;
  var res : i32 = subgroupShuffle(arg_0, arg_1);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = subgroupShuffle_8bfbcd();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupShuffle_8bfbcd();
}

Failed to generate: <dawn>/test/tint/builtins/gen/var/subgroupShuffle/8bfbcd.wgsl:41:8 error: GLSL backend does not support extension 'subgroups'
enable subgroups;
       ^^^^^^^^^


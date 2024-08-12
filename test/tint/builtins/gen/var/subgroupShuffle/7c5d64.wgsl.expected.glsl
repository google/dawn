SKIP: FAILED


enable subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : vec3<f32>;

fn subgroupShuffle_7c5d64() -> vec3<f32> {
  var arg_0 = vec3<f32>(1.0f);
  var arg_1 = 1i;
  var res : vec3<f32> = subgroupShuffle(arg_0, arg_1);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = subgroupShuffle_7c5d64();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupShuffle_7c5d64();
}

Failed to generate: <dawn>/test/tint/builtins/gen/var/subgroupShuffle/7c5d64.wgsl:41:8 error: GLSL backend does not support extension 'subgroups'
enable subgroups;
       ^^^^^^^^^


enable subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : vec3<f32>;

fn subgroupShuffle_7c5d64() -> vec3<f32> {
  var arg_0 = vec3<f32>(1.0f);
  var arg_1 = 1i;
  var res : vec3<f32> = subgroupShuffle(arg_0, arg_1);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = subgroupShuffle_7c5d64();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupShuffle_7c5d64();
}

Failed to generate: <dawn>/test/tint/builtins/gen/var/subgroupShuffle/7c5d64.wgsl:41:8 error: GLSL backend does not support extension 'subgroups'
enable subgroups;
       ^^^^^^^^^


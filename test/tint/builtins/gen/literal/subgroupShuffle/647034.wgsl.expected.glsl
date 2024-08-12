SKIP: FAILED


enable subgroups;
enable subgroups_f16;
enable f16;

@group(0) @binding(0) var<storage, read_write> prevent_dce : vec4<f16>;

fn subgroupShuffle_647034() -> vec4<f16> {
  var res : vec4<f16> = subgroupShuffle(vec4<f16>(1.0h), 1i);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = subgroupShuffle_647034();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupShuffle_647034();
}

Failed to generate: <dawn>/test/tint/builtins/gen/literal/subgroupShuffle/647034.wgsl:41:8 error: GLSL backend does not support extension 'subgroups'
enable subgroups;
       ^^^^^^^^^


enable subgroups;
enable subgroups_f16;
enable f16;

@group(0) @binding(0) var<storage, read_write> prevent_dce : vec4<f16>;

fn subgroupShuffle_647034() -> vec4<f16> {
  var res : vec4<f16> = subgroupShuffle(vec4<f16>(1.0h), 1i);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = subgroupShuffle_647034();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupShuffle_647034();
}

Failed to generate: <dawn>/test/tint/builtins/gen/literal/subgroupShuffle/647034.wgsl:41:8 error: GLSL backend does not support extension 'subgroups'
enable subgroups;
       ^^^^^^^^^


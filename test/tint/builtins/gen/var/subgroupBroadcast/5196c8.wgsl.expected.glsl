SKIP: FAILED


enable chromium_experimental_subgroups;

fn subgroupBroadcast_5196c8() {
  var arg_0 = vec2<f32>(1.0f);
  const arg_1 = 1u;
  var res : vec2<f32> = subgroupBroadcast(arg_0, arg_1);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec2<f32>;

@compute @workgroup_size(1)
fn compute_main() {
  subgroupBroadcast_5196c8();
}

Failed to generate: <dawn>/test/tint/builtins/gen/var/subgroupBroadcast/5196c8.wgsl:38:8 error: GLSL backend does not support extension 'chromium_experimental_subgroups'
enable chromium_experimental_subgroups;
       ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^


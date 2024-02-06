SKIP: FAILED


enable chromium_experimental_subgroups;

fn subgroupBroadcast_b7e93b() {
  var res : vec4<f32> = subgroupBroadcast(vec4<f32>(1.0f), 1u);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec4<f32>;

@compute @workgroup_size(1)
fn compute_main() {
  subgroupBroadcast_b7e93b();
}

Failed to generate: <dawn>/test/tint/builtins/gen/literal/subgroupBroadcast/b7e93b.wgsl:38:8 error: GLSL backend does not support extension 'chromium_experimental_subgroups'
enable chromium_experimental_subgroups;
       ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^


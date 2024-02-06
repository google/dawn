enable chromium_experimental_subgroups;

fn subgroupBroadcast_912ff5() {
  var arg_0 = vec3<f32>(1.0f);
  const arg_1 = 1u;
  var res : vec3<f32> = subgroupBroadcast(arg_0, arg_1);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec3<f32>;

@compute @workgroup_size(1)
fn compute_main() {
  subgroupBroadcast_912ff5();
}

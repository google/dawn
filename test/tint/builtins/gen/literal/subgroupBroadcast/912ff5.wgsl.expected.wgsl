enable chromium_experimental_subgroups;

fn subgroupBroadcast_912ff5() {
  var res : vec3<f32> = subgroupBroadcast(vec3<f32>(1.0f), 1u);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec3<f32>;

@compute @workgroup_size(1)
fn compute_main() {
  subgroupBroadcast_912ff5();
}

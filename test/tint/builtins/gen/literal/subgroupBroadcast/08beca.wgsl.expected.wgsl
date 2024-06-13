enable chromium_experimental_subgroups;

fn subgroupBroadcast_08beca() -> f32 {
  var res : f32 = subgroupBroadcast(1.0f, 1u);
  return res;
}

@group(0) @binding(0) var<storage, read_write> prevent_dce : f32;

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupBroadcast_08beca();
}

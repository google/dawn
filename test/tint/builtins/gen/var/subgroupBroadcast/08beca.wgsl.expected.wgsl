enable chromium_experimental_subgroups;

fn subgroupBroadcast_08beca() -> f32 {
  var arg_0 = 1.0f;
  const arg_1 = 1u;
  var res : f32 = subgroupBroadcast(arg_0, arg_1);
  return res;
}

@group(0) @binding(0) var<storage, read_write> prevent_dce : f32;

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupBroadcast_08beca();
}

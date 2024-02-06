enable chromium_experimental_subgroups;

fn subgroupBroadcast_5196c8() {
  var res : vec2<f32> = subgroupBroadcast(vec2<f32>(1.0f), 1u);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec2<f32>;

@compute @workgroup_size(1)
fn compute_main() {
  subgroupBroadcast_5196c8();
}

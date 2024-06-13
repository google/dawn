enable chromium_experimental_subgroups;

fn subgroupBroadcast_5196c8() -> vec2<f32> {
  var res : vec2<f32> = subgroupBroadcast(vec2<f32>(1.0f), 1u);
  return res;
}

@group(0) @binding(0) var<storage, read_write> prevent_dce : vec2<f32>;

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupBroadcast_5196c8();
}

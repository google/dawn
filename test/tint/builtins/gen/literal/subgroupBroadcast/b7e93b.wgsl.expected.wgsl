enable chromium_experimental_subgroups;

fn subgroupBroadcast_b7e93b() -> vec4<f32> {
  var res : vec4<f32> = subgroupBroadcast(vec4<f32>(1.0f), 1u);
  return res;
}

@group(0) @binding(0) var<storage, read_write> prevent_dce : vec4<f32>;

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupBroadcast_b7e93b();
}

enable subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : vec4<f32>;

fn subgroupMin_bbd9b0() -> vec4<f32> {
  var res : vec4<f32> = subgroupMin(vec4<f32>(1.0f));
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = subgroupMin_bbd9b0();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupMin_bbd9b0();
}

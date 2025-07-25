enable subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : vec2<f32>;

fn subgroupShuffleUp_b58804() -> vec2<f32> {
  var arg_0 = vec2<f32>(1.0f);
  var arg_1 = 1u;
  var res : vec2<f32> = subgroupShuffleUp(arg_0, arg_1);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = subgroupShuffleUp_b58804();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupShuffleUp_b58804();
}

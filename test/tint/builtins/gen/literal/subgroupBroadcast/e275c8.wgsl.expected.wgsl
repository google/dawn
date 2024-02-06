enable chromium_experimental_subgroups;

fn subgroupBroadcast_e275c8() {
  var res : vec3<i32> = subgroupBroadcast(vec3<i32>(1i), 1u);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec3<i32>;

@compute @workgroup_size(1)
fn compute_main() {
  subgroupBroadcast_e275c8();
}

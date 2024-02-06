enable chromium_experimental_subgroups;

fn subgroupBroadcast_3e6879() {
  var arg_0 = vec2<i32>(1i);
  const arg_1 = 1u;
  var res : vec2<i32> = subgroupBroadcast(arg_0, arg_1);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec2<i32>;

@compute @workgroup_size(1)
fn compute_main() {
  subgroupBroadcast_3e6879();
}

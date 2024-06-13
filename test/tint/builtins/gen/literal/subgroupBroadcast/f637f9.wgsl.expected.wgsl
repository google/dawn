enable chromium_experimental_subgroups;

fn subgroupBroadcast_f637f9() -> vec4<i32> {
  var res : vec4<i32> = subgroupBroadcast(vec4<i32>(1i), 1u);
  return res;
}

@group(0) @binding(0) var<storage, read_write> prevent_dce : vec4<i32>;

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupBroadcast_f637f9();
}

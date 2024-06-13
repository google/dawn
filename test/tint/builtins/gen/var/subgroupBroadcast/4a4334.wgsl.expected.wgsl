enable chromium_experimental_subgroups;

fn subgroupBroadcast_4a4334() -> vec2<u32> {
  var arg_0 = vec2<u32>(1u);
  const arg_1 = 1u;
  var res : vec2<u32> = subgroupBroadcast(arg_0, arg_1);
  return res;
}

@group(0) @binding(0) var<storage, read_write> prevent_dce : vec2<u32>;

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupBroadcast_4a4334();
}

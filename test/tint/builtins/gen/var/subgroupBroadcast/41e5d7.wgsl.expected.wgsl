enable chromium_experimental_subgroups;
enable f16;

fn subgroupBroadcast_41e5d7() -> vec3<f16> {
  var arg_0 = vec3<f16>(1.0h);
  const arg_1 = 1u;
  var res : vec3<f16> = subgroupBroadcast(arg_0, arg_1);
  return res;
}

@group(0) @binding(0) var<storage, read_write> prevent_dce : vec3<f16>;

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupBroadcast_41e5d7();
}

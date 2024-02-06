enable chromium_experimental_subgroups;
enable f16;

fn subgroupBroadcast_0f44e2() {
  var arg_0 = vec4<f16>(1.0h);
  const arg_1 = 1u;
  var res : vec4<f16> = subgroupBroadcast(arg_0, arg_1);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec4<f16>;

@compute @workgroup_size(1)
fn compute_main() {
  subgroupBroadcast_0f44e2();
}

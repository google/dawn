enable chromium_experimental_subgroups;
enable f16;

fn subgroupBroadcast_0f44e2() {
  var res : vec4<f16> = subgroupBroadcast(vec4<f16>(1.0h), 1u);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec4<f16>;

@compute @workgroup_size(1)
fn compute_main() {
  subgroupBroadcast_0f44e2();
}

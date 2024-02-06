enable chromium_experimental_subgroups;
enable f16;

fn subgroupBroadcast_13f36c() {
  var res : vec2<f16> = subgroupBroadcast(vec2<f16>(1.0h), 1u);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec2<f16>;

@compute @workgroup_size(1)
fn compute_main() {
  subgroupBroadcast_13f36c();
}

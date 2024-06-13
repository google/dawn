enable chromium_experimental_subgroups;
enable f16;

fn subgroupBroadcast_13f36c() -> vec2<f16> {
  var res : vec2<f16> = subgroupBroadcast(vec2<f16>(1.0h), 1u);
  return res;
}

@group(0) @binding(0) var<storage, read_write> prevent_dce : vec2<f16>;

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupBroadcast_13f36c();
}

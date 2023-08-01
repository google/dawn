enable chromium_experimental_subgroups;

fn subgroupBallot_7e6d0e() {
  var res : vec4<u32> = subgroupBallot();
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec4<u32>;

@compute @workgroup_size(1)
fn compute_main() {
  subgroupBallot_7e6d0e();
}

enable subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : u32;

fn subgroupMax_b58cbf() -> u32 {
  var res : u32 = subgroupMax(1u);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = subgroupMax_b58cbf();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupMax_b58cbf();
}

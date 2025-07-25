enable subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : u32;

fn subgroupMul_4f8ee6() -> u32 {
  var res : u32 = subgroupMul(1u);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = subgroupMul_4f8ee6();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupMul_4f8ee6();
}

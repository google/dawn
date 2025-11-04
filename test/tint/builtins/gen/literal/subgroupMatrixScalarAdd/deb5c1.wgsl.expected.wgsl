enable chromium_experimental_subgroup_matrix;

@group(0) @binding(0) var<storage, read_write> prevent_dce : array<u32, 1024>;

fn subgroupMatrixScalarAdd_deb5c1() -> subgroup_matrix_left<u8, 8, 8> {
  var res : subgroup_matrix_left<u8, 8, 8> = subgroupMatrixScalarAdd(subgroup_matrix_left<u8, 8, 8>(), 8u);
  return res;
}

@compute @workgroup_size(1)
fn compute_main() {
  subgroupMatrixStore(&(prevent_dce), 0, subgroupMatrixScalarAdd_deb5c1(), false, 64);
}

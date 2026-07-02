enable chromium_experimental_subgroup_matrix;

var<workgroup> arg_0 : array<u32, 1024>;

fn subgroupMatrixStore_8b70b3() {
  subgroupMatrixStore(&(arg_0), 1i, subgroup_matrix_right<u8, 8, 8>(), true, 8i);
}

@compute @workgroup_size(1)
fn compute_main() {
  subgroupMatrixStore_8b70b3();
}

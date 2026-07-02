enable chromium_experimental_subgroup_matrix;

var<workgroup> arg_0 : array<u32, 1024>;

fn subgroupMatrixStore_2531d5() {
  subgroupMatrixStore(&(arg_0), 1i, subgroup_matrix_result<u8, 8, 8>(), true, 8i);
}

@compute @workgroup_size(1)
fn compute_main() {
  subgroupMatrixStore_2531d5();
}

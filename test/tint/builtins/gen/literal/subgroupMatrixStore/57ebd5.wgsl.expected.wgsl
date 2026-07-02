enable chromium_experimental_subgroup_matrix;

var<workgroup> arg_0 : array<u32, 1024>;

fn subgroupMatrixStore_57ebd5() {
  subgroupMatrixStore<col_major>(&(arg_0), 1i, subgroup_matrix_right<u32, 8, 8>(), 8u);
}

@compute @workgroup_size(1)
fn compute_main() {
  subgroupMatrixStore_57ebd5();
}

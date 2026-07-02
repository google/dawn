enable chromium_experimental_subgroup_matrix;

var<workgroup> arg_0 : array<i32, 1024>;

fn subgroupMatrixStore_dafc45() {
  subgroupMatrixStore(&(arg_0), 1i, subgroup_matrix_right<i8, 8, 8>(), true, 8u);
}

@compute @workgroup_size(1)
fn compute_main() {
  subgroupMatrixStore_dafc45();
}

enable chromium_experimental_subgroup_matrix;

var<workgroup> arg_0 : array<i32, 1024>;

fn subgroupMatrixStore_1433f9() {
  subgroupMatrixStore(&(arg_0), 1u, subgroup_matrix_left<i32, 8, 8>(), true, 1u);
}

@compute @workgroup_size(1)
fn compute_main() {
  subgroupMatrixStore_1433f9();
}

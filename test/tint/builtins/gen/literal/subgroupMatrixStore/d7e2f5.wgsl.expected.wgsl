enable chromium_experimental_subgroup_matrix;

var<workgroup> arg_0 : array<i32, 1024>;

fn subgroupMatrixStore_d7e2f5() {
  subgroupMatrixStore(&(arg_0), 1u, subgroup_matrix_right<i32, 8, 8>(), true, 8i);
}

@compute @workgroup_size(1)
fn compute_main() {
  subgroupMatrixStore_d7e2f5();
}

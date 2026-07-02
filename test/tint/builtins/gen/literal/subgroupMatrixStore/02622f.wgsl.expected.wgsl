enable chromium_experimental_subgroup_matrix;

var<workgroup> arg_0 : array<i32, 1024>;

fn subgroupMatrixStore_02622f() {
  subgroupMatrixStore(&(arg_0), 1i, subgroup_matrix_left<i32, 8, 8>(), true, 8i);
}

@compute @workgroup_size(1)
fn compute_main() {
  subgroupMatrixStore_02622f();
}

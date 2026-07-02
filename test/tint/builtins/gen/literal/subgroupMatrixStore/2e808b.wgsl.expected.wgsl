enable chromium_experimental_subgroup_matrix;

var<workgroup> arg_0 : array<i32, 1024>;

fn subgroupMatrixStore_2e808b() {
  subgroupMatrixStore(&(arg_0), 1u, subgroup_matrix_result<i32, 8, 8>(), true, 8i);
}

@compute @workgroup_size(1)
fn compute_main() {
  subgroupMatrixStore_2e808b();
}

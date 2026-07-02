enable chromium_experimental_subgroup_matrix;

var<workgroup> arg_0 : array<i32, 1024>;

fn subgroupMatrixStore_a167cf() {
  subgroupMatrixStore<col_major>(&(arg_0), 1i, subgroup_matrix_result<i32, 8, 8>(), 8i);
}

@compute @workgroup_size(1)
fn compute_main() {
  subgroupMatrixStore_a167cf();
}

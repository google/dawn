enable chromium_experimental_subgroup_matrix;

var<workgroup> arg_0 : array<i32, 1024>;

fn subgroupMatrixStore_9fb631() {
  subgroupMatrixStore<col_major>(&(arg_0), 1u, subgroup_matrix_result<i8, 8, 8>(), 8u);
}

@compute @workgroup_size(1)
fn compute_main() {
  subgroupMatrixStore_9fb631();
}

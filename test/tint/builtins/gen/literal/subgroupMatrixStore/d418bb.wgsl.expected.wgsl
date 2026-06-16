enable chromium_experimental_subgroup_matrix;

var<workgroup> arg_0 : array<i32, 1024>;

fn subgroupMatrixStore_d418bb() {
  subgroupMatrixStore<col_major>(&(arg_0), 1u, subgroup_matrix_left<i32, 8, 8>(), 8u);
}

@compute @workgroup_size(1)
fn compute_main() {
  subgroupMatrixStore_d418bb();
}

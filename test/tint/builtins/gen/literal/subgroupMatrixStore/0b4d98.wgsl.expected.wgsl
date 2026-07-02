enable chromium_experimental_subgroup_matrix;

var<workgroup> arg_0 : array<i32, 1024>;

fn subgroupMatrixStore_0b4d98() {
  subgroupMatrixStore<col_major>(&(arg_0), 1u, subgroup_matrix_right<i8, 8, 8>(), 8i);
}

@compute @workgroup_size(1)
fn compute_main() {
  subgroupMatrixStore_0b4d98();
}

enable chromium_experimental_subgroup_matrix;

var<workgroup> arg_0 : array<u32, 1024>;

fn subgroupMatrixStore_7a6d6d() {
  subgroupMatrixStore<col_major>(&(arg_0), 1u, subgroup_matrix_left<u8, 8, 8>(), 8u);
}

@compute @workgroup_size(1)
fn compute_main() {
  subgroupMatrixStore_7a6d6d();
}

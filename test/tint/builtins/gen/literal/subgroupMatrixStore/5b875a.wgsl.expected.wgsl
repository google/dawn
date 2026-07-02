enable chromium_experimental_subgroup_matrix;

var<workgroup> arg_0 : array<u32, 1024>;

fn subgroupMatrixStore_5b875a() {
  subgroupMatrixStore<row_major>(&(arg_0), 1i, subgroup_matrix_left<u32, 8, 8>(), 8u);
}

@compute @workgroup_size(1)
fn compute_main() {
  subgroupMatrixStore_5b875a();
}

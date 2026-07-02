enable chromium_experimental_subgroup_matrix;

var<workgroup> arg_0 : array<f32, 1024>;

fn subgroupMatrixStore_2c367b() {
  subgroupMatrixStore<row_major>(&(arg_0), 1i, subgroup_matrix_result<f32, 8, 8>(), 8i);
}

@compute @workgroup_size(1)
fn compute_main() {
  subgroupMatrixStore_2c367b();
}

enable chromium_experimental_subgroup_matrix;

var<workgroup> arg_0 : array<f32, 1024>;

fn subgroupMatrixStore_982f8d() {
  subgroupMatrixStore(&(arg_0), 1i, subgroup_matrix_result<f32, 8, 8>(), true, 8i);
}

@compute @workgroup_size(1)
fn compute_main() {
  subgroupMatrixStore_982f8d();
}

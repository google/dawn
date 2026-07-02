enable chromium_experimental_subgroup_matrix;

var<workgroup> arg_0 : array<f32, 1024>;

fn subgroupMatrixStore_2fe626() {
  subgroupMatrixStore(&(arg_0), 1i, subgroup_matrix_left<f32, 8, 8>(), true, 8u);
}

@compute @workgroup_size(1)
fn compute_main() {
  subgroupMatrixStore_2fe626();
}

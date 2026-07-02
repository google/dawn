enable chromium_experimental_subgroup_matrix;

var<workgroup> arg_0 : array<f32, 1024>;

fn subgroupMatrixStore_0e3035() {
  subgroupMatrixStore(&(arg_0), 1u, subgroup_matrix_right<f32, 8, 8>(), true, 8i);
}

@compute @workgroup_size(1)
fn compute_main() {
  subgroupMatrixStore_0e3035();
}

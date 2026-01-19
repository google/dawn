enable chromium_experimental_subgroup_matrix;

var<workgroup> arg_0 : array<u32, 1024>;

fn subgroupMatrixStore_314acd() {
  subgroupMatrixStore(&(arg_0), 1u, subgroup_matrix_right<u32, 8, 8>(), true, 8u);
}

@compute @workgroup_size(1)
fn compute_main() {
  subgroupMatrixStore_314acd();
}

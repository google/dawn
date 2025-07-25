enable chromium_experimental_subgroup_matrix;
enable f16;

var<workgroup> arg_0 : array<f16, 1024>;

fn subgroupMatrixStore_45de0c() {
  subgroupMatrixStore(&(arg_0), 1u, subgroup_matrix_left<f16, 8, 8>(), true, 8u);
}

@compute @workgroup_size(1)
fn compute_main() {
  subgroupMatrixStore_45de0c();
}

enable chromium_experimental_subgroup_matrix;
enable f16;

var<workgroup> arg_0 : array<f16, 1024>;

fn subgroupMatrixStore_53598f() {
  subgroupMatrixStore(&(arg_0), 1u, subgroup_matrix_right<f16, 8, 8>(), true, 8i);
}

@compute @workgroup_size(1)
fn compute_main() {
  subgroupMatrixStore_53598f();
}

enable chromium_experimental_subgroup_matrix;

var<workgroup> arg_0 : array<u32, 1024>;

fn subgroupMatrixStore_f8df8d() {
  subgroupMatrixStore(&(arg_0), 1u, subgroup_matrix_right<u8, 8, 8>(), true, 8i);
}

@compute @workgroup_size(1)
fn compute_main() {
  subgroupMatrixStore_f8df8d();
}

enable chromium_experimental_subgroup_matrix;

var<workgroup> arg_0 : array<u32, 1024>;

fn subgroupMatrixStore_43b7fd() {
  subgroupMatrixStore(&(arg_0), 1u, subgroup_matrix_right<u8, 8, 8>(), true, 8u);
}

@compute @workgroup_size(1)
fn compute_main() {
  subgroupMatrixStore_43b7fd();
}

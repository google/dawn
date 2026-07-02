enable chromium_experimental_subgroup_matrix;
enable f16;

var<workgroup> arg_0 : array<f16, 1024>;

fn subgroupMatrixStore_b65c98() {
  subgroupMatrixStore<row_major>(&(arg_0), 1i, subgroup_matrix_result<f16, 8, 8>(), 8u);
}

@compute @workgroup_size(1)
fn compute_main() {
  subgroupMatrixStore_b65c98();
}

enable chromium_experimental_subgroup_matrix;
enable f16;

var<workgroup> arg_0 : array<f16, 1024>;

fn subgroupMatrixStore_d86a3e() {
  subgroupMatrixStore<row_major>(&(arg_0), 1u, subgroup_matrix_result<f16, 8, 8>(), 8i);
}

@compute @workgroup_size(1)
fn compute_main() {
  subgroupMatrixStore_d86a3e();
}

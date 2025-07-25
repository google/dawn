enable chromium_experimental_subgroup_matrix;
enable f16;

var<workgroup> arg_0 : array<f16, 1024>;

fn subgroupMatrixStore_840822() {
  var arg_1 = 1u;
  var arg_2 = subgroup_matrix_result<f16, 8, 8>();
  const arg_3 = true;
  var arg_4 = 8u;
  subgroupMatrixStore(&(arg_0), arg_1, arg_2, arg_3, arg_4);
}

@compute @workgroup_size(1)
fn compute_main() {
  subgroupMatrixStore_840822();
}

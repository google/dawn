enable chromium_experimental_subgroup_matrix;
enable f16;

var<workgroup> arg_0 : array<f16, 1024>;

fn subgroupMatrixStore_519c95() {
  var arg_1 = 1u;
  var arg_2 = subgroup_matrix_left<f16, 8, 8>();
  var arg_3 = 8u;
  subgroupMatrixStore<row_major>(&(arg_0), arg_1, arg_2, arg_3);
}

@compute @workgroup_size(1)
fn compute_main() {
  subgroupMatrixStore_519c95();
}

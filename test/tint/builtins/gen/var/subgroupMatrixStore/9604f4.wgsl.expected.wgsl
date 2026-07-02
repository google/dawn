enable chromium_experimental_subgroup_matrix;

var<workgroup> arg_0 : array<f32, 1024>;

fn subgroupMatrixStore_9604f4() {
  var arg_1 = 1i;
  var arg_2 = subgroup_matrix_right<f32, 8, 8>();
  var arg_3 = 8i;
  subgroupMatrixStore<col_major>(&(arg_0), arg_1, arg_2, arg_3);
}

@compute @workgroup_size(1)
fn compute_main() {
  subgroupMatrixStore_9604f4();
}

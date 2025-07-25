enable chromium_experimental_subgroup_matrix;

@group(0) @binding(0) var<storage, read_write> prevent_dce : array<i32, 1024>;

fn subgroupMatrixMultiply_d755b4() -> subgroup_matrix_result<i8, 8, 8> {
  var arg_0 = subgroup_matrix_left<u8, 8, 8>();
  var arg_1 = subgroup_matrix_right<u8, 8, 8>();
  var res : subgroup_matrix_result<i8, 8, 8> = subgroupMatrixMultiply<i8>(arg_0, arg_1);
  return res;
}

@compute @workgroup_size(1)
fn compute_main() {
  subgroupMatrixStore(&(prevent_dce), 0, subgroupMatrixMultiply_d755b4(), false, 64);
}

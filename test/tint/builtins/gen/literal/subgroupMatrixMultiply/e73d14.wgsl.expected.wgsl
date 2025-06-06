SKIP: FAILED

enable chromium_experimental_subgroup_matrix;

@group(0) @binding(0) var<storage, read_write> prevent_dce : array<i32, 1024>;

fn subgroupMatrixMultiply_e73d14() -> subgroup_matrix_result<i8, 8, 8> {
  var res : subgroup_matrix_result<i8, 8, 8> = subgroupMatrixMultiply<i8>(subgroup_matrix_left<i32, 8, 8>(), subgroup_matrix_right<i32, 8, 8>());
  return res;
}

@compute @workgroup_size(1)
fn compute_main() {
  subgroupMatrixStore(&(prevent_dce), 0, subgroupMatrixMultiply_e73d14(), false, 64);
}

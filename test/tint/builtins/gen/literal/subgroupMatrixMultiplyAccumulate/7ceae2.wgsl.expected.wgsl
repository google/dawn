enable chromium_experimental_subgroup_matrix;
enable f16;

@group(0) @binding(0) var<storage, read_write> prevent_dce : array<f16, 1024>;

fn subgroupMatrixMultiplyAccumulate_7ceae2() -> subgroup_matrix_result<f16, 8, 8> {
  var res : subgroup_matrix_result<f16, 8, 8> = subgroupMatrixMultiplyAccumulate(subgroup_matrix_left<i32, 8, 8>(), subgroup_matrix_right<i32, 8, 8>(), subgroup_matrix_result<f16, 8, 8>());
  return res;
}

@compute @workgroup_size(1)
fn compute_main() {
  subgroupMatrixStore(&(prevent_dce), 0, subgroupMatrixMultiplyAccumulate_7ceae2(), false, 64);
}

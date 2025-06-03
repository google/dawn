enable chromium_experimental_subgroup_matrix;

@group(0) @binding(0) var<storage, read_write> prevent_dce : array<f32, 1024>;

fn subgroupMatrixMultiplyAccumulate_b23f3e() -> subgroup_matrix_result<f32, 8, 8> {
  var res : subgroup_matrix_result<f32, 8, 8> = subgroupMatrixMultiplyAccumulate(subgroup_matrix_left<i8, 8, 8>(), subgroup_matrix_right<i8, 8, 8>(), subgroup_matrix_result<f32, 8, 8>());
  return res;
}

@compute @workgroup_size(1)
fn compute_main() {
  subgroupMatrixStore(&(prevent_dce), 0, subgroupMatrixMultiplyAccumulate_b23f3e(), false, 64);
}

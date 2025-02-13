enable chromium_experimental_subgroup_matrix;

@group(0) @binding(0) var<storage, read_write> prevent_dce : array<i32, 1024>;

fn subgroupMatrixMultiplyAccumulate_676ca8() -> subgroup_matrix_result<i32, 8, 8> {
  var res : subgroup_matrix_result<i32, 8, 8> = subgroupMatrixMultiplyAccumulate(subgroup_matrix_left<u32, 8, 8>(), subgroup_matrix_right<u32, 8, 8>(), subgroup_matrix_result<i32, 8, 8>());
  return res;
}

@compute @workgroup_size(1)
fn compute_main() {
  subgroupMatrixStore(&(prevent_dce), 0, subgroupMatrixMultiplyAccumulate_676ca8(), false, 64);
}

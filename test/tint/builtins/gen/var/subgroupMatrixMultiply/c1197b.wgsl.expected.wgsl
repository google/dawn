enable chromium_experimental_subgroup_matrix, subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : array<f32, 1024>;

fn subgroupMatrixMultiply_c1197b() -> subgroup_matrix_result<bf16, 8, 8> {
  var arg_0 = subgroup_matrix_left<bf16, 8, 8>();
  var arg_1 = subgroup_matrix_right<bf16, 8, 8>();
  var res : subgroup_matrix_result<bf16, 8, 8> = subgroupMatrixMultiply<bf16>(arg_0, arg_1);
  return res;
}

@compute @workgroup_size(1)
fn compute_main() {
  subgroupMatrixStore<row_major>(&(prevent_dce), 0, subgroupMatrixMultiply_c1197b(), 16);
}

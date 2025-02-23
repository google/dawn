enable chromium_experimental_subgroup_matrix;
enable f16;

@group(0) @binding(0) var<storage, read_write> prevent_dce : array<f16, 1024>;

fn subgroupMatrixMultiply_db4ee8() -> subgroup_matrix_result<f16, 8, 8> {
  var arg_0 = subgroup_matrix_left<u32, 8, 8>();
  var arg_1 = subgroup_matrix_right<u32, 8, 8>();
  var res : subgroup_matrix_result<f16, 8, 8> = subgroupMatrixMultiply<f16>(arg_0, arg_1);
  return res;
}

@compute @workgroup_size(1)
fn compute_main() {
  subgroupMatrixStore(&(prevent_dce), 0, subgroupMatrixMultiply_db4ee8(), false, 64);
}

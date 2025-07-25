enable chromium_experimental_subgroup_matrix;

@group(0) @binding(0) var<storage, read_write> prevent_dce : array<u32, 1024>;

fn subgroupMatrixMultiply_3e9b04() -> subgroup_matrix_result<u32, 8, 8> {
  var arg_0 = subgroup_matrix_left<i32, 8, 8>();
  var arg_1 = subgroup_matrix_right<i32, 8, 8>();
  var res : subgroup_matrix_result<u32, 8, 8> = subgroupMatrixMultiply<u32>(arg_0, arg_1);
  return res;
}

@compute @workgroup_size(1)
fn compute_main() {
  subgroupMatrixStore(&(prevent_dce), 0, subgroupMatrixMultiply_3e9b04(), false, 64);
}

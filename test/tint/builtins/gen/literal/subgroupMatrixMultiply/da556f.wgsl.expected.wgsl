enable chromium_experimental_subgroup_matrix;

@group(0) @binding(0) var<storage, read_write> prevent_dce : array<u32, 1024>;

fn subgroupMatrixMultiply_da556f() -> subgroup_matrix_result<u32, 8, 8> {
  var res : subgroup_matrix_result<u32, 8, 8> = subgroupMatrixMultiply<u32>(subgroup_matrix_left<u32, 8, 8>(), subgroup_matrix_right<u32, 8, 8>());
  return res;
}

@compute @workgroup_size(1)
fn compute_main() {
  subgroupMatrixStore(&(prevent_dce), 0, subgroupMatrixMultiply_da556f(), false, 64);
}

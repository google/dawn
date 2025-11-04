enable chromium_experimental_subgroup_matrix;

@group(0) @binding(0) var<storage, read_write> prevent_dce : array<i32, 1024>;

fn subgroupMatrixScalarAdd_9b357f() -> subgroup_matrix_result<i8, 8, 8> {
  var res : subgroup_matrix_result<i8, 8, 8> = subgroupMatrixScalarAdd(subgroup_matrix_result<i8, 8, 8>(), 8i);
  return res;
}

@compute @workgroup_size(1)
fn compute_main() {
  subgroupMatrixStore(&(prevent_dce), 0, subgroupMatrixScalarAdd_9b357f(), false, 64);
}

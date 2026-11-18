enable chromium_experimental_subgroup_matrix, subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : array<f32, 1024>;

fn subgroupMatrixScalarMultiply_45d761() -> subgroup_matrix_right<bf16, 8, 8> {
  var res : subgroup_matrix_right<bf16, 8, 8> = subgroupMatrixScalarMultiply(subgroup_matrix_right<bf16, 8, 8>(), 8.0f);
  return res;
}

@compute @workgroup_size(1)
fn compute_main() {
  subgroupMatrixStore<row_major>(&(prevent_dce), 0, subgroupMatrixScalarMultiply_45d761(), 16);
}

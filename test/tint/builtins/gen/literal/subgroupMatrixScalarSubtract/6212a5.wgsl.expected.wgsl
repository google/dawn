enable chromium_experimental_subgroup_matrix, subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : array<f32, 1024>;

fn subgroupMatrixScalarSubtract_6212a5() -> subgroup_matrix_left<bf16, 8, 8> {
  var res : subgroup_matrix_left<bf16, 8, 8> = subgroupMatrixScalarSubtract(subgroup_matrix_left<bf16, 8, 8>(), 8.0f);
  return res;
}

@compute @workgroup_size(1)
fn compute_main() {
  subgroupMatrixStore<row_major>(&(prevent_dce), 0, subgroupMatrixScalarSubtract_6212a5(), 16);
}

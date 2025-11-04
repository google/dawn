enable chromium_experimental_subgroup_matrix;

@group(0) @binding(0) var<storage, read_write> prevent_dce : array<f32, 1024>;

fn subgroupMatrixScalarAdd_7f3869() -> subgroup_matrix_left<f32, 8, 8> {
  var res : subgroup_matrix_left<f32, 8, 8> = subgroupMatrixScalarAdd(subgroup_matrix_left<f32, 8, 8>(), 8.0f);
  return res;
}

@compute @workgroup_size(1)
fn compute_main() {
  subgroupMatrixStore(&(prevent_dce), 0, subgroupMatrixScalarAdd_7f3869(), false, 64);
}

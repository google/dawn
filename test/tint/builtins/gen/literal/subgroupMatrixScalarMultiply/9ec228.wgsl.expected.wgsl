enable chromium_experimental_subgroup_matrix;

@group(0) @binding(0) var<storage, read_write> prevent_dce : array<f32, 1024>;

fn subgroupMatrixScalarMultiply_9ec228() -> subgroup_matrix_result<f32, 8, 8> {
  var res : subgroup_matrix_result<f32, 8, 8> = subgroupMatrixScalarMultiply(subgroup_matrix_result<f32, 8, 8>(), 8.0f);
  return res;
}

@compute @workgroup_size(1)
fn compute_main() {
  subgroupMatrixStore(&(prevent_dce), 0, subgroupMatrixScalarMultiply_9ec228(), false, 64);
}

enable chromium_experimental_subgroup_matrix;
enable f16;

@group(0) @binding(0) var<storage, read_write> prevent_dce : array<f16, 1024>;

fn subgroupMatrixScalarAdd_ee2a47() -> subgroup_matrix_result<f16, 8, 8> {
  var res : subgroup_matrix_result<f16, 8, 8> = subgroupMatrixScalarAdd(subgroup_matrix_result<f16, 8, 8>(), 8.0h);
  return res;
}

@compute @workgroup_size(1)
fn compute_main() {
  subgroupMatrixStore(&(prevent_dce), 0, subgroupMatrixScalarAdd_ee2a47(), false, 64);
}

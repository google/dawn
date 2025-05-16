enable chromium_experimental_subgroup_matrix;
enable f16;

@group(0) @binding(0) var<storage, read_write> prevent_dce : array<f16, 1024>;

var<workgroup> arg_0 : array<f16, 1024>;

fn subgroupMatrixLoad_a13603() -> subgroup_matrix_left<f16, 8, 8> {
  var res : subgroup_matrix_left<f16, 8, 8> = subgroupMatrixLoad<subgroup_matrix_left<f16, 8, 8>>(&(arg_0), 1u, true, 1u);
  return res;
}

@compute @workgroup_size(1)
fn compute_main() {
  subgroupMatrixStore(&(prevent_dce), 0, subgroupMatrixLoad_a13603(), false, 64);
}

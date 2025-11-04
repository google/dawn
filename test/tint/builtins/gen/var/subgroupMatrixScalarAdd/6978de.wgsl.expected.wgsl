enable chromium_experimental_subgroup_matrix;

@group(0) @binding(0) var<storage, read_write> prevent_dce : array<f32, 1024>;

fn subgroupMatrixScalarAdd_6978de() -> subgroup_matrix_result<f32, 8, 8> {
  var arg_0 = subgroup_matrix_result<f32, 8, 8>();
  var arg_1 = 8.0f;
  var res : subgroup_matrix_result<f32, 8, 8> = subgroupMatrixScalarAdd(arg_0, arg_1);
  return res;
}

@compute @workgroup_size(1)
fn compute_main() {
  subgroupMatrixStore(&(prevent_dce), 0, subgroupMatrixScalarAdd_6978de(), false, 64);
}

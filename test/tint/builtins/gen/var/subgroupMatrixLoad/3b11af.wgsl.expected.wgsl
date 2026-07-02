enable chromium_experimental_subgroup_matrix;
enable f16;

@group(0) @binding(0) var<storage, read_write> prevent_dce : array<f16, 1024>;

var<workgroup> arg_0 : array<f16, 1024>;

fn subgroupMatrixLoad_3b11af() -> subgroup_matrix_right<f16, 8, 8> {
  var arg_1 = 1i;
  var arg_2 = 8i;
  var res : subgroup_matrix_right<f16, 8, 8> = subgroupMatrixLoad<subgroup_matrix_right<f16, 8, 8>, row_major>(&(arg_0), arg_1, arg_2);
  return res;
}

@compute @workgroup_size(1)
fn compute_main() {
  subgroupMatrixStore(&(prevent_dce), 0, subgroupMatrixLoad_3b11af(), false, 8);
}

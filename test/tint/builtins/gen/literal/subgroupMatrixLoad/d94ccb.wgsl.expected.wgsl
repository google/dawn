enable chromium_experimental_subgroup_matrix;

@group(0) @binding(0) var<storage, read_write> prevent_dce : array<f32, 1024>;

var<workgroup> arg_0 : array<f32, 1024>;

fn subgroupMatrixLoad_d94ccb() -> subgroup_matrix_left<f32, 8, 8> {
  var res : subgroup_matrix_left<f32, 8, 8> = subgroupMatrixLoad<subgroup_matrix_left<f32, 8, 8>, row_major>(&(arg_0), 1u, 8u);
  return res;
}

@compute @workgroup_size(1)
fn compute_main() {
  subgroupMatrixStore(&(prevent_dce), 0, subgroupMatrixLoad_d94ccb(), false, 64);
}

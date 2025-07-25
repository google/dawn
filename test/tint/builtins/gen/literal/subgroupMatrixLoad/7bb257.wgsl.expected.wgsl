enable chromium_experimental_subgroup_matrix;

@group(0) @binding(0) var<storage, read_write> prevent_dce : array<i32, 1024>;

var<workgroup> arg_0 : array<i32, 1024>;

fn subgroupMatrixLoad_7bb257() -> subgroup_matrix_left<i32, 8, 8> {
  var res : subgroup_matrix_left<i32, 8, 8> = subgroupMatrixLoad<subgroup_matrix_left<i32, 8, 8>>(&(arg_0), 1u, true, 8u);
  return res;
}

@compute @workgroup_size(1)
fn compute_main() {
  subgroupMatrixStore(&(prevent_dce), 0, subgroupMatrixLoad_7bb257(), false, 64);
}

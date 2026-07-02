enable chromium_experimental_subgroup_matrix;

@group(0) @binding(0) var<storage, read_write> prevent_dce : array<i32, 1024>;

var<workgroup> arg_0 : array<i32, 1024>;

fn subgroupMatrixLoad_1db25d() -> subgroup_matrix_right<i8, 8, 8> {
  var res : subgroup_matrix_right<i8, 8, 8> = subgroupMatrixLoad<subgroup_matrix_right<i8, 8, 8>, col_major>(&(arg_0), 1u, 8i);
  return res;
}

@compute @workgroup_size(1)
fn compute_main() {
  subgroupMatrixStore(&(prevent_dce), 0, subgroupMatrixLoad_1db25d(), false, 8);
}

enable chromium_experimental_subgroup_matrix;

@group(0) @binding(0) var<storage, read_write> prevent_dce : array<i32, 1024>;

var<workgroup> arg_0 : array<i32, 1024>;

fn subgroupMatrixLoad_1cdc2f() -> subgroup_matrix_right<i8, 8, 8> {
  var arg_1 = 1i;
  var arg_2 = 8u;
  var res : subgroup_matrix_right<i8, 8, 8> = subgroupMatrixLoad<subgroup_matrix_right<i8, 8, 8>, col_major>(&(arg_0), arg_1, arg_2);
  return res;
}

@compute @workgroup_size(1)
fn compute_main() {
  subgroupMatrixStore(&(prevent_dce), 0, subgroupMatrixLoad_1cdc2f(), false, 8);
}

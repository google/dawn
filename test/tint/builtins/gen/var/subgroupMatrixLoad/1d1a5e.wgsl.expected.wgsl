enable chromium_experimental_subgroup_matrix;

@group(0) @binding(0) var<storage, read_write> prevent_dce : array<i32, 1024>;

struct SB_RO {
  arg_0 : array<i32, 1024>,
}

@group(0) @binding(1) var<storage, read> sb_ro : SB_RO;

fn subgroupMatrixLoad_1d1a5e() -> subgroup_matrix_left<i32, 8, 8> {
  var arg_1 = 1u;
  var arg_2 = 8i;
  var res : subgroup_matrix_left<i32, 8, 8> = subgroupMatrixLoad<subgroup_matrix_left<i32, 8, 8>, row_major>(&(sb_ro.arg_0), arg_1, arg_2);
  return res;
}

@compute @workgroup_size(1)
fn compute_main() {
  subgroupMatrixStore(&(prevent_dce), 0, subgroupMatrixLoad_1d1a5e(), false, 8);
}

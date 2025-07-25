enable chromium_experimental_subgroup_matrix;

@group(0) @binding(0) var<storage, read_write> prevent_dce : array<i32, 1024>;

struct SB_RO {
  arg_0 : array<i32, 1024>,
}

@group(0) @binding(1) var<storage, read> sb_ro : SB_RO;

fn subgroupMatrixLoad_865a3c() -> subgroup_matrix_right<i32, 8, 8> {
  var arg_1 = 1u;
  const arg_2 = true;
  var arg_3 = 8u;
  var res : subgroup_matrix_right<i32, 8, 8> = subgroupMatrixLoad<subgroup_matrix_right<i32, 8, 8>>(&(sb_ro.arg_0), arg_1, arg_2, arg_3);
  return res;
}

@compute @workgroup_size(1)
fn compute_main() {
  subgroupMatrixStore(&(prevent_dce), 0, subgroupMatrixLoad_865a3c(), false, 64);
}

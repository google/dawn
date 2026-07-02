enable chromium_experimental_subgroup_matrix;
enable f16;

@group(0) @binding(0) var<storage, read_write> prevent_dce : array<f16, 1024>;

struct SB_RO {
  arg_0 : array<f16, 1024>,
}

@group(0) @binding(1) var<storage, read> sb_ro : SB_RO;

fn subgroupMatrixLoad_bb1164() -> subgroup_matrix_right<f16, 8, 8> {
  var arg_1 = 1i;
  var arg_2 = 8u;
  var res : subgroup_matrix_right<f16, 8, 8> = subgroupMatrixLoad<subgroup_matrix_right<f16, 8, 8>, col_major>(&(sb_ro.arg_0), arg_1, arg_2);
  return res;
}

@compute @workgroup_size(1)
fn compute_main() {
  subgroupMatrixStore(&(prevent_dce), 0, subgroupMatrixLoad_bb1164(), false, 8);
}

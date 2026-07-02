enable chromium_experimental_subgroup_matrix;

@group(0) @binding(0) var<storage, read_write> prevent_dce : array<u32, 1024>;

struct SB_RW {
  arg_0 : array<u32, 1024>,
}

@group(0) @binding(1) var<storage, read_write> sb_rw : SB_RW;

fn subgroupMatrixLoad_16e406() -> subgroup_matrix_left<u8, 8, 8> {
  var res : subgroup_matrix_left<u8, 8, 8> = subgroupMatrixLoad<subgroup_matrix_left<u8, 8, 8>, col_major>(&(sb_rw.arg_0), 1i, 8i);
  return res;
}

@compute @workgroup_size(1)
fn compute_main() {
  subgroupMatrixStore(&(prevent_dce), 0, subgroupMatrixLoad_16e406(), false, 8);
}

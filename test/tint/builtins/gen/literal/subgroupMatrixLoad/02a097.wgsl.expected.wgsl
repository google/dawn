enable chromium_experimental_subgroup_matrix;

@group(0) @binding(0) var<storage, read_write> prevent_dce : array<u32, 1024>;

struct SB_RO {
  arg_0 : array<u32, 1024>,
}

@group(0) @binding(1) var<storage, read> sb_ro : SB_RO;

fn subgroupMatrixLoad_02a097() -> subgroup_matrix_right<u32, 8, 8> {
  var res : subgroup_matrix_right<u32, 8, 8> = subgroupMatrixLoad<subgroup_matrix_right<u32, 8, 8>, col_major>(&(sb_ro.arg_0), 1u, 8i);
  return res;
}

@compute @workgroup_size(1)
fn compute_main() {
  subgroupMatrixStore(&(prevent_dce), 0, subgroupMatrixLoad_02a097(), false, 8);
}

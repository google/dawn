enable chromium_experimental_subgroup_matrix;

@group(0) @binding(0) var<storage, read_write> prevent_dce : array<u32, 1024>;

struct SB_RW {
  arg_0 : array<u32, 1024>,
}

@group(0) @binding(1) var<storage, read_write> sb_rw : SB_RW;

fn subgroupMatrixLoad_99d2a2() -> subgroup_matrix_result<u32, 8, 8> {
  var res : subgroup_matrix_result<u32, 8, 8> = subgroupMatrixLoad<subgroup_matrix_result<u32, 8, 8>>(&(sb_rw.arg_0), 1u, true, 8u);
  return res;
}

@compute @workgroup_size(1)
fn compute_main() {
  subgroupMatrixStore(&(prevent_dce), 0, subgroupMatrixLoad_99d2a2(), false, 64);
}

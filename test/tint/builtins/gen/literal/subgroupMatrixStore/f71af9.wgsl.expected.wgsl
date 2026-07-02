enable chromium_experimental_subgroup_matrix;

struct SB_RW {
  arg_0 : array<i32, 1024>,
}

@group(0) @binding(0) var<storage, read_write> sb_rw : SB_RW;

fn subgroupMatrixStore_f71af9() {
  subgroupMatrixStore(&(sb_rw.arg_0), 1i, subgroup_matrix_left<i32, 8, 8>(), true, 8u);
}

@compute @workgroup_size(1)
fn compute_main() {
  subgroupMatrixStore_f71af9();
}

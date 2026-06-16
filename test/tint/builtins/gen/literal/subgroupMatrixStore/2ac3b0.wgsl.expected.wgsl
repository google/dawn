enable chromium_experimental_subgroup_matrix;

struct SB_RW {
  arg_0 : array<u32, 1024>,
}

@group(0) @binding(0) var<storage, read_write> sb_rw : SB_RW;

fn subgroupMatrixStore_2ac3b0() {
  subgroupMatrixStore<col_major>(&(sb_rw.arg_0), 1u, subgroup_matrix_left<u32, 8, 8>(), 8u);
}

@compute @workgroup_size(1)
fn compute_main() {
  subgroupMatrixStore_2ac3b0();
}

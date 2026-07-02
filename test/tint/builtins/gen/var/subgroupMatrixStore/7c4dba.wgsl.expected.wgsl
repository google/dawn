enable chromium_experimental_subgroup_matrix;

struct SB_RW {
  arg_0 : array<u32, 1024>,
}

@group(0) @binding(0) var<storage, read_write> sb_rw : SB_RW;

fn subgroupMatrixStore_7c4dba() {
  var arg_1 = 1i;
  var arg_2 = subgroup_matrix_left<u8, 8, 8>();
  var arg_3 = 8u;
  subgroupMatrixStore<row_major>(&(sb_rw.arg_0), arg_1, arg_2, arg_3);
}

@compute @workgroup_size(1)
fn compute_main() {
  subgroupMatrixStore_7c4dba();
}

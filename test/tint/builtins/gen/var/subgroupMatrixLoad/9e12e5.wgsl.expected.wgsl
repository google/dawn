enable chromium_experimental_subgroup_matrix;

@group(0) @binding(0) var<storage, read_write> prevent_dce : array<f32, 1024>;

struct SB_RW {
  arg_0 : array<f32, 1024>,
}

@group(0) @binding(1) var<storage, read_write> sb_rw : SB_RW;

fn subgroupMatrixLoad_9e12e5() -> subgroup_matrix_left<f32, 8, 8> {
  var arg_1 = 1i;
  var arg_2 = 8u;
  var res : subgroup_matrix_left<f32, 8, 8> = subgroupMatrixLoad<subgroup_matrix_left<f32, 8, 8>, row_major>(&(sb_rw.arg_0), arg_1, arg_2);
  return res;
}

@compute @workgroup_size(1)
fn compute_main() {
  subgroupMatrixStore(&(prevent_dce), 0, subgroupMatrixLoad_9e12e5(), false, 8);
}

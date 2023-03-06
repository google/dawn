struct SB_RW {
  arg_0 : atomic<i32>,
}

@group(0) @binding(0) var<storage, read_write> sb_rw : SB_RW;

fn atomicOr_8d96a0() {
  var arg_1 = 1i;
  var res : i32 = atomicOr(&(sb_rw.arg_0), arg_1);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : i32;

@fragment
fn fragment_main() {
  atomicOr_8d96a0();
}

@compute @workgroup_size(1)
fn compute_main() {
  atomicOr_8d96a0();
}

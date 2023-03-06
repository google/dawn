struct SB_RW {
  arg_0 : atomic<i32>,
}

@group(0) @binding(0) var<storage, read_write> sb_rw : SB_RW;

fn atomicSub_051100() {
  var res : i32 = atomicSub(&(sb_rw.arg_0), 1i);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : i32;

@fragment
fn fragment_main() {
  atomicSub_051100();
}

@compute @workgroup_size(1)
fn compute_main() {
  atomicSub_051100();
}

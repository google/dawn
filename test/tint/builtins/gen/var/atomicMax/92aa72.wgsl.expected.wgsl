struct SB_RW {
  arg_0 : atomic<i32>,
}

@group(0) @binding(0) var<storage, read_write> sb_rw : SB_RW;

fn atomicMax_92aa72() {
  var arg_1 = 1i;
  var res : i32 = atomicMax(&(sb_rw.arg_0), arg_1);
}

@fragment
fn fragment_main() {
  atomicMax_92aa72();
}

@compute @workgroup_size(1)
fn compute_main() {
  atomicMax_92aa72();
}

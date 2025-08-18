struct SB_RW_atomic {
  arg_0 : atomic<i32>,
}

@group(0u) @binding(0u) var<storage, read_write> sb_rw : SB_RW_atomic;

fn atomicSub_051100() {
  var res : i32 = 0i;
  res = atomicSub(&(sb_rw.arg_0), 1i);
}

@fragment
fn fragment_main() {
  atomicSub_051100();
}

@compute @workgroup_size(1u, 1u, 1u)
fn compute_main() {
  atomicSub_051100();
}

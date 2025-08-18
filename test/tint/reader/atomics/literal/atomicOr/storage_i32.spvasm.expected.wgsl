struct SB_RW_atomic {
  arg_0 : atomic<i32>,
}

@group(0u) @binding(0u) var<storage, read_write> sb_rw : SB_RW_atomic;

fn atomicOr_8d96a0() {
  var res : i32 = 0i;
  res = atomicOr(&(sb_rw.arg_0), 1i);
}

@fragment
fn fragment_main() {
  atomicOr_8d96a0();
}

@compute @workgroup_size(1u, 1u, 1u)
fn compute_main() {
  atomicOr_8d96a0();
}

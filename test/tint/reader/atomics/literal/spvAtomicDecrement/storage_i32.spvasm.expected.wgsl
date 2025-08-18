struct SB_RW_atomic {
  arg_0 : atomic<i32>,
}

@group(0u) @binding(0u) var<storage, read_write> sb_rw : SB_RW_atomic;

fn atomicAdd_d32fe4() {
  var res : i32 = 0i;
  res = atomicSub(&(sb_rw.arg_0), 1i);
}

@fragment
fn fragment_main() {
  atomicAdd_d32fe4();
}

@compute @workgroup_size(1u, 1u, 1u)
fn compute_main() {
  atomicAdd_d32fe4();
}

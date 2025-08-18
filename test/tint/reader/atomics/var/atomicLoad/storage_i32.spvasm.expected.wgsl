struct SB_RW_atomic {
  arg_0 : atomic<i32>,
}

@group(0u) @binding(0u) var<storage, read_write> sb_rw : SB_RW_atomic;

fn atomicLoad_0806ad() {
  var res : i32 = 0i;
  res = atomicLoad(&(sb_rw.arg_0));
}

@fragment
fn fragment_main() {
  atomicLoad_0806ad();
}

@compute @workgroup_size(1u, 1u, 1u)
fn compute_main() {
  atomicLoad_0806ad();
}

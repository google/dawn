struct SB_RW_atomic {
  arg_0 : atomic<u32>,
}

@group(0u) @binding(0u) var<storage, read_write> sb_rw : SB_RW_atomic;

fn atomicAnd_85a8d9() {
  var res : u32 = 0u;
  res = atomicAnd(&(sb_rw.arg_0), 1u);
}

@fragment
fn fragment_main() {
  atomicAnd_85a8d9();
}

@compute @workgroup_size(1u, 1u, 1u)
fn compute_main() {
  atomicAnd_85a8d9();
}

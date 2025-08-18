struct SB_RW_atomic {
  arg_0 : atomic<u32>,
}

@group(0u) @binding(0u) var<storage, read_write> sb_rw : SB_RW_atomic;

fn atomicOr_5e95d4() {
  var arg_1 : u32 = 0u;
  var res : u32 = 0u;
  arg_1 = 1u;
  res = atomicOr(&(sb_rw.arg_0), arg_1);
}

@fragment
fn fragment_main() {
  atomicOr_5e95d4();
}

@compute @workgroup_size(1u, 1u, 1u)
fn compute_main() {
  atomicOr_5e95d4();
}

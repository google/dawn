struct SB_RW_atomic {
  arg_0 : atomic<i32>,
}

@group(0u) @binding(0u) var<storage, read_write> sb_rw : SB_RW_atomic;

fn atomicStore_d1e9a6() {
  atomicStore(&(sb_rw.arg_0), 1i);
}

@fragment
fn fragment_main() {
  atomicStore_d1e9a6();
}

@compute @workgroup_size(1u, 1u, 1u)
fn compute_main() {
  atomicStore_d1e9a6();
}

struct SB_RW_atomic {
  arg_0 : atomic<u32>,
}

@group(0u) @binding(0u) var<storage, read_write> sb_rw : SB_RW_atomic;

fn atomicStore_cdc29e() {
  atomicStore(&(sb_rw.arg_0), 1u);
}

@fragment
fn fragment_main() {
  atomicStore_cdc29e();
}

@compute @workgroup_size(1u, 1u, 1u)
fn compute_main() {
  atomicStore_cdc29e();
}

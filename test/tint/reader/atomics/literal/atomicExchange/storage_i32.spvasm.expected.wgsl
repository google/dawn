struct SB_RW_atomic {
  arg_0 : atomic<i32>,
}

@group(0u) @binding(0u) var<storage, read_write> sb_rw : SB_RW_atomic;

fn atomicExchange_f2e22f() {
  var res : i32 = 0i;
  res = atomicExchange(&(sb_rw.arg_0), 1i);
}

@fragment
fn fragment_main() {
  atomicExchange_f2e22f();
}

@compute @workgroup_size(1u, 1u, 1u)
fn compute_main() {
  atomicExchange_f2e22f();
}

struct SB_RW_atomic {
  arg_0 : atomic<u32>,
}

@group(0u) @binding(0u) var<storage, read_write> sb_rw : SB_RW_atomic;

struct S {
  old_value : u32,
  exchanged : bool,
}

fn atomicCompareExchangeWeak_63d8e6() {
  var res : S = S();
  let v = atomicCompareExchangeWeak(&(sb_rw.arg_0), 1u, 1u).old_value;
  res = S(v, (v == 1u));
}

@fragment
fn fragment_main() {
  atomicCompareExchangeWeak_63d8e6();
}

@compute @workgroup_size(1u, 1u, 1u)
fn compute_main() {
  atomicCompareExchangeWeak_63d8e6();
}

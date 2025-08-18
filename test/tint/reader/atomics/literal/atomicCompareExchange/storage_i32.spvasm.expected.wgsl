struct SB_RW_atomic {
  arg_0 : atomic<i32>,
}

@group(0u) @binding(0u) var<storage, read_write> sb_rw : SB_RW_atomic;

struct S {
  old_value : i32,
  exchanged : bool,
}

fn atomicCompareExchangeWeak_1bd40a() {
  var res : S = S();
  let v = atomicCompareExchangeWeak(&(sb_rw.arg_0), 1i, 1i).old_value;
  res = S(v, (v == 1i));
}

@fragment
fn fragment_main() {
  atomicCompareExchangeWeak_1bd40a();
}

@compute @workgroup_size(1u, 1u, 1u)
fn compute_main() {
  atomicCompareExchangeWeak_1bd40a();
}

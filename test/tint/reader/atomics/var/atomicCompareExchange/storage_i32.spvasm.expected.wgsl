struct SB_RW_atomic {
  arg_0 : atomic<i32>,
}

@group(0u) @binding(0u) var<storage, read_write> sb_rw : SB_RW_atomic;

struct S {
  old_value : i32,
  exchanged : bool,
}

fn atomicCompareExchangeWeak_1bd40a() {
  var arg_1 : i32 = 0i;
  var arg_2 : i32 = 0i;
  var res : S = S();
  arg_1 = 1i;
  arg_2 = 1i;
  let v = arg_2;
  let v_1 = atomicCompareExchangeWeak(&(sb_rw.arg_0), arg_1, v).old_value;
  res = S(v_1, (v_1 == v));
}

@fragment
fn fragment_main() {
  atomicCompareExchangeWeak_1bd40a();
}

@compute @workgroup_size(1u, 1u, 1u)
fn compute_main() {
  atomicCompareExchangeWeak_1bd40a();
}

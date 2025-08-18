struct SB_RW_atomic {
  arg_0 : atomic<u32>,
}

@group(0u) @binding(0u) var<storage, read_write> sb_rw : SB_RW_atomic;

struct S {
  old_value : u32,
  exchanged : bool,
}

fn atomicCompareExchangeWeak_63d8e6() {
  var arg_1 : u32 = 0u;
  var arg_2 : u32 = 0u;
  var res : S = S();
  arg_1 = 1u;
  arg_2 = 1u;
  let v = arg_2;
  let v_1 = atomicCompareExchangeWeak(&(sb_rw.arg_0), arg_1, v).old_value;
  res = S(v_1, (v_1 == v));
}

@fragment
fn fragment_main() {
  atomicCompareExchangeWeak_63d8e6();
}

@compute @workgroup_size(1u, 1u, 1u)
fn compute_main() {
  atomicCompareExchangeWeak_63d8e6();
}

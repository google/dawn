var<workgroup> arg_0 : atomic<i32>;

struct S {
  old_value : i32,
  exchanged : bool,
}

fn atomicCompareExchangeWeak_e88938() {
  var arg_1 : i32 = 0i;
  var arg_2 : i32 = 0i;
  var res : S = S();
  arg_1 = 1i;
  arg_2 = 1i;
  let v = arg_2;
  let v_1 = atomicCompareExchangeWeak(&(arg_0), arg_1, v).old_value;
  res = S(v_1, (v_1 == v));
}

fn compute_main_inner(local_invocation_index : u32) {
  atomicStore(&(arg_0), 0i);
  workgroupBarrier();
  atomicCompareExchangeWeak_e88938();
}

@compute @workgroup_size(1u, 1u, 1u)
fn compute_main(@builtin(local_invocation_index) local_invocation_index_1 : u32) {
  compute_main_inner(local_invocation_index_1);
}

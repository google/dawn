var<workgroup> arg_0 : atomic<i32>;

struct S {
  old_value : i32,
  exchanged : bool,
}

fn atomicCompareExchangeWeak_e88938() {
  var res : S = S();
  let v = atomicCompareExchangeWeak(&(arg_0), 1i, 1i).old_value;
  res = S(v, (v == 1i));
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

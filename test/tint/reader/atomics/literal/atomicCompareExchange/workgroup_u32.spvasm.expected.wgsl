var<workgroup> arg_0 : atomic<u32>;

struct S {
  old_value : u32,
  exchanged : bool,
}

fn atomicCompareExchangeWeak_83580d() {
  var res : S = S();
  let v = atomicCompareExchangeWeak(&(arg_0), 1u, 1u).old_value;
  res = S(v, (v == 1u));
}

fn compute_main_inner(local_invocation_index : u32) {
  atomicStore(&(arg_0), 0u);
  workgroupBarrier();
  atomicCompareExchangeWeak_83580d();
}

@compute @workgroup_size(1u, 1u, 1u)
fn compute_main(@builtin(local_invocation_index) local_invocation_index_1 : u32) {
  compute_main_inner(local_invocation_index_1);
}

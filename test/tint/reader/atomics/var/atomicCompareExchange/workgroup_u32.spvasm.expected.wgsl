var<workgroup> arg_0 : atomic<u32>;

struct S {
  old_value : u32,
  exchanged : bool,
}

fn atomicCompareExchangeWeak_83580d() {
  var arg_1 : u32 = 0u;
  var arg_2 : u32 = 0u;
  var res : S = S();
  arg_1 = 1u;
  arg_2 = 1u;
  let v = arg_2;
  let v_1 = atomicCompareExchangeWeak(&(arg_0), arg_1, v).old_value;
  res = S(v_1, (v_1 == v));
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

var<workgroup> arg_0 : atomic<u32>;

fn atomicMin_69d383() {
  var res : u32 = 0u;
  res = atomicMin(&(arg_0), 1u);
}

fn compute_main_inner(local_invocation_index : u32) {
  atomicStore(&(arg_0), 0u);
  workgroupBarrier();
  atomicMin_69d383();
}

@compute @workgroup_size(1u, 1u, 1u)
fn compute_main(@builtin(local_invocation_index) local_invocation_index_1 : u32) {
  compute_main_inner(local_invocation_index_1);
}

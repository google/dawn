var<workgroup> arg_0 : atomic<u32>;

fn atomicAdd_d5db1d() {
  var res : u32 = 0u;
  res = atomicAdd(&(arg_0), 1u);
}

fn compute_main_inner(local_invocation_index : u32) {
  atomicStore(&(arg_0), 0u);
  workgroupBarrier();
  atomicAdd_d5db1d();
}

@compute @workgroup_size(1u, 1u, 1u)
fn compute_main(@builtin(local_invocation_index) local_invocation_index_1 : u32) {
  compute_main_inner(local_invocation_index_1);
}

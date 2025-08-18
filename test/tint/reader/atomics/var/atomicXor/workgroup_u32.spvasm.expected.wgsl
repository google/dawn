var<workgroup> arg_0 : atomic<u32>;

fn atomicXor_c8e6be() {
  var arg_1 : u32 = 0u;
  var res : u32 = 0u;
  arg_1 = 1u;
  res = atomicXor(&(arg_0), arg_1);
}

fn compute_main_inner(local_invocation_index : u32) {
  atomicStore(&(arg_0), 0u);
  workgroupBarrier();
  atomicXor_c8e6be();
}

@compute @workgroup_size(1u, 1u, 1u)
fn compute_main(@builtin(local_invocation_index) local_invocation_index_1 : u32) {
  compute_main_inner(local_invocation_index_1);
}

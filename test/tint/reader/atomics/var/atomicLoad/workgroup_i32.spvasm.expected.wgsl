var<workgroup> arg_0 : atomic<i32>;

fn atomicLoad_afcc03() {
  var res : i32 = 0i;
  res = atomicLoad(&(arg_0));
}

fn compute_main_inner(local_invocation_index : u32) {
  atomicStore(&(arg_0), 0i);
  workgroupBarrier();
  atomicLoad_afcc03();
}

@compute @workgroup_size(1u, 1u, 1u)
fn compute_main(@builtin(local_invocation_index) local_invocation_index_1 : u32) {
  compute_main_inner(local_invocation_index_1);
}

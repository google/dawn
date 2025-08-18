var<workgroup> arg_0 : atomic<i32>;

fn atomicMax_a89cc3() {
  var arg_1 : i32 = 0i;
  var res : i32 = 0i;
  arg_1 = 1i;
  res = atomicMax(&(arg_0), arg_1);
}

fn compute_main_inner(local_invocation_index : u32) {
  atomicStore(&(arg_0), 0i);
  workgroupBarrier();
  atomicMax_a89cc3();
}

@compute @workgroup_size(1u, 1u, 1u)
fn compute_main(@builtin(local_invocation_index) local_invocation_index_1 : u32) {
  compute_main_inner(local_invocation_index_1);
}

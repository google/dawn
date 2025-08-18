struct S_atomic {
  x : i32,
  a : atomic<u32>,
  b : atomic<u32>,
}

var<workgroup> wg : S_atomic;

fn compute_main_inner(local_invocation_index : u32) {
  wg.x = 0i;
  atomicStore(&(wg.a), 0u);
  atomicStore(&(wg.b), 0u);
  workgroupBarrier();
  atomicStore(&(wg.a), 1u);
  atomicStore(&(wg.b), 2u);
}

@compute @workgroup_size(1u, 1u, 1u)
fn compute_main(@builtin(local_invocation_index) local_invocation_index_1 : u32) {
  compute_main_inner(local_invocation_index_1);
}

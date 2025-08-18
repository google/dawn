struct S0_atomic {
  x : i32,
  a : atomic<u32>,
  y : i32,
  z : i32,
}

struct S1_atomic {
  x : i32,
  a : S0_atomic,
  y : i32,
  z : i32,
}

struct S2_atomic {
  x : i32,
  y : i32,
  z : i32,
  a : S1_atomic,
}

var<workgroup> wg : S2_atomic;

fn compute_main_inner(local_invocation_index : u32) {
  wg.x = 0i;
  wg.y = 0i;
  wg.z = 0i;
  wg.a.x = 0i;
  wg.a.a.x = 0i;
  atomicStore(&(wg.a.a.a), 0u);
  wg.a.a.y = 0i;
  wg.a.a.z = 0i;
  wg.a.y = 0i;
  wg.a.z = 0i;
  workgroupBarrier();
  atomicStore(&(wg.a.a.a), 1u);
}

@compute @workgroup_size(1u, 1u, 1u)
fn compute_main(@builtin(local_invocation_index) local_invocation_index_1 : u32) {
  compute_main_inner(local_invocation_index_1);
}

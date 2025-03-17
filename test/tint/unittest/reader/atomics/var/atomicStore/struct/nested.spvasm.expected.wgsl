struct S0_atomic {
  x : i32,
  a : atomic<u32>,
  y : i32,
  z : i32,
}

struct S0 {
  x : i32,
  a : u32,
  y : i32,
  z : i32,
}

struct S1_atomic {
  x : i32,
  a : S0_atomic,
  y : i32,
  z : i32,
}

struct S1 {
  x : i32,
  a : S0,
  y : i32,
  z : i32,
}

struct S2_atomic {
  x : i32,
  y : i32,
  z : i32,
  a : S1_atomic,
}

struct S2 {
  x : i32,
  y : i32,
  z : i32,
  a : S1,
}

var<private> local_invocation_index_1 : u32;

var<workgroup> wg : S2_atomic;

fn compute_main_inner(local_invocation_index_2 : u32) {
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
  return;
}

fn compute_main_1() {
  let x_44 = local_invocation_index_1;
  compute_main_inner(x_44);
  return;
}

@compute @workgroup_size(1i, 1i, 1i)
fn compute_main(@builtin(local_invocation_index) local_invocation_index_1_param : u32) {
  local_invocation_index_1 = local_invocation_index_1_param;
  compute_main_1();
}

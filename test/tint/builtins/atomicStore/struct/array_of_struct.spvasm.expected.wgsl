struct S_atomic {
  /* @offset(0) */
  x : i32,
  /* @offset(4) */
  a : atomic<u32>,
  /* @offset(8) */
  y : u32,
}

struct S {
  /* @offset(0) */
  x : i32,
  /* @offset(4) */
  a : u32,
  /* @offset(8) */
  y : u32,
}

type Arr = array<S, 10u>;

var<private> local_invocation_index_1 : u32;

var<workgroup> wg : array<S_atomic, 10u>;

fn compute_main_inner(local_invocation_index : u32) {
  var idx : u32 = 0u;
  idx = local_invocation_index;
  loop {
    let x_23 : u32 = idx;
    if (!((x_23 < 10u))) {
      break;
    }
    let x_28 : u32 = idx;
    wg[x_28].x = 0i;
    atomicStore(&(wg[x_28].a), 0u);
    wg[x_28].y = 0u;

    continuing {
      let x_41 : u32 = idx;
      idx = (x_41 + 1u);
    }
  }
  workgroupBarrier();
  atomicStore(&(wg[4i].a), 1u);
  return;
}

fn compute_main_1() {
  let x_53 : u32 = local_invocation_index_1;
  compute_main_inner(x_53);
  return;
}

@compute @workgroup_size(1i, 1i, 1i)
fn compute_main(@builtin(local_invocation_index) local_invocation_index_1_param : u32) {
  local_invocation_index_1 = local_invocation_index_1_param;
  compute_main_1();
}

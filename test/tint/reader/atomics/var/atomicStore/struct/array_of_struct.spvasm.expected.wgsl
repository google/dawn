struct S_atomic {
  x : i32,
  a : atomic<u32>,
  y : u32,
}

var<workgroup> wg : array<S_atomic, 10u>;

fn compute_main_inner(local_invocation_index : u32) {
  var idx : u32 = 0u;
  idx = local_invocation_index;
  loop {
    if (!((idx < 10u))) {
      break;
    }
    let v = idx;
    wg[v].x = 0i;
    atomicStore(&(wg[v].a), 0u);
    wg[v].y = 0u;

    continuing {
      idx = (idx + 1u);
    }
  }
  workgroupBarrier();
  atomicStore(&(wg[4i].a), 1u);
}

@compute @workgroup_size(1u, 1u, 1u)
fn compute_main(@builtin(local_invocation_index) local_invocation_index_1 : u32) {
  compute_main_inner(local_invocation_index_1);
}

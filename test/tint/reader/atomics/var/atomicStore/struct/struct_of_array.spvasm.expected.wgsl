struct S_atomic {
  x : i32,
  a : array<atomic<u32>, 10u>,
  y : u32,
}

var<workgroup> wg : S_atomic;

fn compute_main_inner(local_invocation_index : u32) {
  var idx : u32 = 0u;
  wg.x = 0i;
  wg.y = 0u;
  idx = local_invocation_index;
  loop {
    if (!((idx < 10u))) {
      break;
    }
    atomicStore(&(wg.a[idx]), 0u);

    continuing {
      idx = (idx + 1u);
    }
  }
  workgroupBarrier();
  atomicStore(&(wg.a[4i]), 1u);
}

@compute @workgroup_size(1u, 1u, 1u)
fn compute_main(@builtin(local_invocation_index) local_invocation_index_1 : u32) {
  compute_main_inner(local_invocation_index_1);
}

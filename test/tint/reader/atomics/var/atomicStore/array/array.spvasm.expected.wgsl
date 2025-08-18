var<workgroup> wg : array<atomic<u32>, 4u>;

fn compute_main_inner(local_invocation_index : u32) {
  var idx : u32 = 0u;
  idx = local_invocation_index;
  loop {
    if (!((idx < 4u))) {
      break;
    }
    atomicStore(&(wg[idx]), 0u);

    continuing {
      idx = (idx + 1u);
    }
  }
  workgroupBarrier();
  atomicStore(&(wg[1i]), 1u);
}

@compute @workgroup_size(1u, 1u, 1u)
fn compute_main(@builtin(local_invocation_index) local_invocation_index_1 : u32) {
  compute_main_inner(local_invocation_index_1);
}

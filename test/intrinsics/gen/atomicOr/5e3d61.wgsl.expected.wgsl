var<workgroup> arg_0 : atomic<u32>;

fn atomicOr_5e3d61() {
  var res : u32 = atomicOr(&(arg_0), 1u);
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  atomicOr_5e3d61();
}

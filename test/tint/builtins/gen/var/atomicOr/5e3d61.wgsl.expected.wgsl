var<workgroup> arg_0 : atomic<u32>;

fn atomicOr_5e3d61() {
  var arg_1 = 1u;
  var res : u32 = atomicOr(&(arg_0), arg_1);
}

@compute @workgroup_size(1)
fn compute_main() {
  atomicOr_5e3d61();
}

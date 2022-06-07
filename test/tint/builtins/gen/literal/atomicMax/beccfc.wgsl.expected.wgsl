var<workgroup> arg_0 : atomic<u32>;

fn atomicMax_beccfc() {
  var res : u32 = atomicMax(&(arg_0), 1u);
}

@compute @workgroup_size(1)
fn compute_main() {
  atomicMax_beccfc();
}

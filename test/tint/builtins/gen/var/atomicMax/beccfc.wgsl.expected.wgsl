var<workgroup> arg_0 : atomic<u32>;

fn atomicMax_beccfc() {
  var arg_1 = 1u;
  var res : u32 = atomicMax(&(arg_0), arg_1);
}

@compute @workgroup_size(1)
fn compute_main() {
  atomicMax_beccfc();
}

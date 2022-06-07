var<workgroup> arg_0 : atomic<u32>;

fn atomicMin_69d383() {
  var arg_1 = 1u;
  var res : u32 = atomicMin(&(arg_0), arg_1);
}

@compute @workgroup_size(1)
fn compute_main() {
  atomicMin_69d383();
}

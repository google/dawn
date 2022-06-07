var<workgroup> arg_0 : atomic<u32>;

fn atomicAnd_34edd3() {
  var arg_1 = 1u;
  var res : u32 = atomicAnd(&(arg_0), arg_1);
}

@compute @workgroup_size(1)
fn compute_main() {
  atomicAnd_34edd3();
}

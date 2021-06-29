var<workgroup> arg_0 : atomic<u32>;

fn atomicAnd_34edd3() {
  var res : u32 = atomicAnd(&(arg_0), 1u);
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  atomicAnd_34edd3();
}

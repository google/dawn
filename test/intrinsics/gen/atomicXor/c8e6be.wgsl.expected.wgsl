var<workgroup> arg_0 : atomic<u32>;

fn atomicXor_c8e6be() {
  var res : u32 = atomicXor(&(arg_0), 1u);
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  atomicXor_c8e6be();
}

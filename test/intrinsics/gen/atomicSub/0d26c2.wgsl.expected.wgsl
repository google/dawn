var<workgroup> arg_0 : atomic<u32>;

fn atomicSub_0d26c2() {
  var res : u32 = atomicSub(&(arg_0), 1u);
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  atomicSub_0d26c2();
}

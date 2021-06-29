var<workgroup> arg_0 : atomic<i32>;

fn atomicAnd_45a819() {
  var res : i32 = atomicAnd(&(arg_0), 1);
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  atomicAnd_45a819();
}

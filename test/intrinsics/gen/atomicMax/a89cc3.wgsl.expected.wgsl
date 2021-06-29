var<workgroup> arg_0 : atomic<i32>;

fn atomicMax_a89cc3() {
  var res : i32 = atomicMax(&(arg_0), 1);
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  atomicMax_a89cc3();
}

var<workgroup> arg_0 : atomic<i32>;

fn atomicLoad_afcc03() {
  var res : i32 = atomicLoad(&(arg_0));
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  atomicLoad_afcc03();
}

var<workgroup> arg_0 : atomic<i32>;

fn atomicStore_8bea94() {
  atomicStore(&(arg_0), 1);
}

[[stage(compute)]]
fn compute_main() {
  atomicStore_8bea94();
}

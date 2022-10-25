var<workgroup> arg_0 : atomic<i32>;

fn atomicAnd_45a819() {
  var arg_1 = 1i;
  var res : i32 = atomicAnd(&(arg_0), arg_1);
}

@compute @workgroup_size(1)
fn compute_main() {
  atomicAnd_45a819();
}

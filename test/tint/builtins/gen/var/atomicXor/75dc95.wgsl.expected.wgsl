var<workgroup> arg_0 : atomic<i32>;

fn atomicXor_75dc95() {
  var arg_1 = 1i;
  var res : i32 = atomicXor(&(arg_0), arg_1);
}

@compute @workgroup_size(1)
fn compute_main() {
  atomicXor_75dc95();
}

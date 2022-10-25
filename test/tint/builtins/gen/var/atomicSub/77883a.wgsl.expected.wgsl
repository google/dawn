var<workgroup> arg_0 : atomic<i32>;

fn atomicSub_77883a() {
  var arg_1 = 1i;
  var res : i32 = atomicSub(&(arg_0), arg_1);
}

@compute @workgroup_size(1)
fn compute_main() {
  atomicSub_77883a();
}

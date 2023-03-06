var<workgroup> arg_0 : atomic<i32>;

fn atomicXor_75dc95() {
  var res : i32 = atomicXor(&(arg_0), 1i);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : i32;

@compute @workgroup_size(1)
fn compute_main() {
  atomicXor_75dc95();
}

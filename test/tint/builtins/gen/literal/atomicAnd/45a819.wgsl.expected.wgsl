var<workgroup> arg_0 : atomic<i32>;

fn atomicAnd_45a819() {
  var res : i32 = atomicAnd(&(arg_0), 1i);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : i32;

@compute @workgroup_size(1)
fn compute_main() {
  atomicAnd_45a819();
}

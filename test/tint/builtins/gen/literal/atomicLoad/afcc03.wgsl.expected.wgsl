var<workgroup> arg_0 : atomic<i32>;

fn atomicLoad_afcc03() {
  var res : i32 = atomicLoad(&(arg_0));
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : i32;

@compute @workgroup_size(1)
fn compute_main() {
  atomicLoad_afcc03();
}

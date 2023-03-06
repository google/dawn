var<workgroup> arg_0 : atomic<i32>;

fn atomicExchange_e114ba() {
  var res : i32 = atomicExchange(&(arg_0), 1i);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : i32;

@compute @workgroup_size(1)
fn compute_main() {
  atomicExchange_e114ba();
}

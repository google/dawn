struct SB_RW {
  arg_0 : atomic<i32>,
}

@group(0) @binding(0) var<storage, read_write> sb_rw : SB_RW;

fn atomicLoad_0806ad() {
  var res : i32 = atomicLoad(&(sb_rw.arg_0));
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : i32;

@fragment
fn fragment_main() {
  atomicLoad_0806ad();
}

@compute @workgroup_size(1)
fn compute_main() {
  atomicLoad_0806ad();
}

struct SB_RW_atomic {
  arg_0 : atomic<u32>,
}

@group(0u) @binding(0u) var<storage, read_write> sb_rw : SB_RW_atomic;

fn atomicAdd_8a199a() {
  var arg_1 : u32 = 0u;
  var res : u32 = 0u;
  arg_1 = 1u;
  res = atomicAdd(&(sb_rw.arg_0), arg_1);
}

@fragment
fn fragment_main() {
  atomicAdd_8a199a();
}

@compute @workgroup_size(1u, 1u, 1u)
fn compute_main() {
  atomicAdd_8a199a();
}

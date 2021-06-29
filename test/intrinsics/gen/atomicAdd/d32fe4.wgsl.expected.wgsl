[[block]]
struct SB_RW {
  arg_0 : atomic<i32>;
};

[[group(0), binding(0)]] var<storage, read_write> sb_rw : SB_RW;

fn atomicAdd_d32fe4() {
  var res : i32 = atomicAdd(&(sb_rw.arg_0), 1);
}

[[stage(fragment)]]
fn fragment_main() {
  atomicAdd_d32fe4();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  atomicAdd_d32fe4();
}

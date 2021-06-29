[[block]]
struct SB_RW {
  arg_0 : atomic<i32>;
};

[[group(0), binding(0)]] var<storage, read_write> sb_rw : SB_RW;

fn atomicOr_8d96a0() {
  var res : i32 = atomicOr(&(sb_rw.arg_0), 1);
}

[[stage(fragment)]]
fn fragment_main() {
  atomicOr_8d96a0();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  atomicOr_8d96a0();
}

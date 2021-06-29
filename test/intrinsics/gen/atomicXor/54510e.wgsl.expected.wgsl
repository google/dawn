[[block]]
struct SB_RW {
  arg_0 : atomic<u32>;
};

[[group(0), binding(0)]] var<storage, read_write> sb_rw : SB_RW;

fn atomicXor_54510e() {
  var res : u32 = atomicXor(&(sb_rw.arg_0), 1u);
}

[[stage(fragment)]]
fn fragment_main() {
  atomicXor_54510e();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  atomicXor_54510e();
}

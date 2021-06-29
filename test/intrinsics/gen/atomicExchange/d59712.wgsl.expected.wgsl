[[block]]
struct SB_RW {
  arg_0 : atomic<u32>;
};

[[group(0), binding(0)]] var<storage, read_write> sb_rw : SB_RW;

fn atomicExchange_d59712() {
  var res : u32 = atomicExchange(&(sb_rw.arg_0), 1u);
}

[[stage(fragment)]]
fn fragment_main() {
  atomicExchange_d59712();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  atomicExchange_d59712();
}

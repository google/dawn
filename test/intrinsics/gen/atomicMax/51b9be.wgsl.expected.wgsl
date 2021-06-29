[[block]]
struct SB_RW {
  arg_0 : atomic<u32>;
};

[[group(0), binding(0)]] var<storage, read_write> sb_rw : SB_RW;

fn atomicMax_51b9be() {
  var res : u32 = atomicMax(&(sb_rw.arg_0), 1u);
}

[[stage(fragment)]]
fn fragment_main() {
  atomicMax_51b9be();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  atomicMax_51b9be();
}

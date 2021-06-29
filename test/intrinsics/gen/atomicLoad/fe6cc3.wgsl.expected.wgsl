[[block]]
struct SB_RW {
  arg_0 : atomic<u32>;
};

[[group(0), binding(0)]] var<storage, read_write> sb_rw : SB_RW;

fn atomicLoad_fe6cc3() {
  var res : u32 = atomicLoad(&(sb_rw.arg_0));
}

[[stage(fragment)]]
fn fragment_main() {
  atomicLoad_fe6cc3();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  atomicLoad_fe6cc3();
}

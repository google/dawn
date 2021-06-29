[[block]]
struct SB_RW {
  arg_0 : atomic<i32>;
};

[[group(0), binding(0)]] var<storage, read_write> sb_rw : SB_RW;

fn atomicExchange_f2e22f() {
  var res : i32 = atomicExchange(&(sb_rw.arg_0), 1);
}

[[stage(fragment)]]
fn fragment_main() {
  atomicExchange_f2e22f();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  atomicExchange_f2e22f();
}

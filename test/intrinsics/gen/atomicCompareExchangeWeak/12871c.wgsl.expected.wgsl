[[block]]
struct SB_RW {
  arg_0 : atomic<i32>;
};

[[group(0), binding(0)]] var<storage, read_write> sb_rw : SB_RW;

fn atomicCompareExchangeWeak_12871c() {
  var res : vec2<i32> = atomicCompareExchangeWeak(&(sb_rw.arg_0), 1, 1);
}

[[stage(fragment)]]
fn fragment_main() {
  atomicCompareExchangeWeak_12871c();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  atomicCompareExchangeWeak_12871c();
}

[[block]]
struct SB_RW {
  arg_0 : atomic<u32>;
};

[[group(0), binding(0)]] var<storage, read_write> sb_rw : SB_RW;

fn atomicCompareExchangeWeak_6673da() {
  var res : vec2<u32> = atomicCompareExchangeWeak(&(sb_rw.arg_0), 1u, 1u);
}

[[stage(fragment)]]
fn fragment_main() {
  atomicCompareExchangeWeak_6673da();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  atomicCompareExchangeWeak_6673da();
}

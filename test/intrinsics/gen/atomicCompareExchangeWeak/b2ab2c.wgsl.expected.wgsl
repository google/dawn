var<workgroup> arg_0 : atomic<u32>;

fn atomicCompareExchangeWeak_b2ab2c() {
  var res : vec2<u32> = atomicCompareExchangeWeak(&(arg_0), 1u, 1u);
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  atomicCompareExchangeWeak_b2ab2c();
}

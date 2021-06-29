var<workgroup> arg_0 : atomic<i32>;

fn atomicCompareExchangeWeak_89ea3b() {
  var res : vec2<i32> = atomicCompareExchangeWeak(&(arg_0), 1, 1);
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  atomicCompareExchangeWeak_89ea3b();
}

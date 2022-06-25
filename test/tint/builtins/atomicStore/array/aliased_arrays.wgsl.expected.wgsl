type A0 = atomic<u32>;

type A1 = array<A0, 1>;

type A2 = array<A1, 2>;

type A3 = array<A2, 3>;

var<workgroup> wg : A3;

@compute @workgroup_size(1)
fn compute_main() {
  atomicStore(&(wg[2][1][0]), 1u);
}

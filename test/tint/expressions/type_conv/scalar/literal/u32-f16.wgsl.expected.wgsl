enable f16;

var<private> u : f16 = f16(u32(1u));

@compute @workgroup_size(1)
fn main() {
  _ = u;
}

enable f16;

var<private> u : bool = bool(f16(1.0h));

@compute @workgroup_size(1)
fn main() {
  _ = u;
}

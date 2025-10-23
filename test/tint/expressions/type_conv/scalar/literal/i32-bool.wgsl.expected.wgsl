var<private> u : bool = bool(i32(1i));

@compute @workgroup_size(1)
fn main() {
  _ = u;
}

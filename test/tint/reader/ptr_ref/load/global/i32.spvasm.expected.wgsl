var<private> I : i32 = 0i;

@compute @workgroup_size(1u, 1u, 1u)
fn main() {
  _ = (I + 1i);
}

var<private> I : i32 = 0i;

@compute @workgroup_size(1u, 1u, 1u)
fn main() {
  I = 123i;
  I = ((100i + 20i) + 3i);
}

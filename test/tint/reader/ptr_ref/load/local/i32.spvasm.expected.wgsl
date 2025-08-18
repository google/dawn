@compute @workgroup_size(1u, 1u, 1u)
fn main() {
  var i : i32 = 0i;
  i = 123i;
  _ = (i + 1i);
}

struct S {
  i : i32,
}

@compute @workgroup_size(1u, 1u, 1u)
fn main() {
  var V : S;
  V.i = 5i;
}

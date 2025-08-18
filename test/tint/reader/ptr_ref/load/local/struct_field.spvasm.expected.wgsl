struct S {
  i : i32,
}

@compute @workgroup_size(1u, 1u, 1u)
fn main() {
  var i : i32;
  var V : S;
  i = V.i;
}

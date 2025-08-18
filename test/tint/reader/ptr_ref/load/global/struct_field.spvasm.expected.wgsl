struct S {
  i : i32,
}

var<private> V : S;

@compute @workgroup_size(1u, 1u, 1u)
fn main() {
  var i : i32;
  i = V.i;
}

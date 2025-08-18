struct S {
  i : i32,
}

var<private> V : S;

@compute @workgroup_size(1u, 1u, 1u)
fn main() {
  V.i = 5i;
}

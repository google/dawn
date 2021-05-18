struct S {
  i : i32;
};

var<private> V : S;

[[stage(compute)]]
fn main() {
  let i : i32 = V.i;
}

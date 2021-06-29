struct S {
  i : i32;
};

var<private> V : S;

[[stage(compute), workgroup_size(1)]]
fn main() {
  let i : i32 = V.i;
}

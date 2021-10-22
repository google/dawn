struct S {
  a : i32;
  b : f32;
};

[[stage(compute), workgroup_size(1)]]
fn main() {
  var v : S;
  _ = v;
}

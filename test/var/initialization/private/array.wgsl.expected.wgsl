var<private> v : array<i32, 3>;

[[stage(compute), workgroup_size(1)]]
fn main() {
  ignore(v);
}

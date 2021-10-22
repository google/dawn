var<workgroup> v : array<i32, 3>;

[[stage(compute), workgroup_size(1)]]
fn main() {
  _ = v;
}

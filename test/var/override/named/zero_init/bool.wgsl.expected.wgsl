[[override]] let o : bool = bool();

[[stage(compute), workgroup_size(1)]]
fn main() {
  _ = o;
}

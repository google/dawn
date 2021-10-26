[[override]] let o : u32 = u32();

[[stage(compute), workgroup_size(1)]]
fn main() {
  _ = o;
}

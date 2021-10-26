[[override(1234)]] let o : u32;

[[stage(compute), workgroup_size(1)]]
fn main() {
  _ = o;
}

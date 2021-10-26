[[override(1234)]] let o : u32 = 1u;

[[stage(compute), workgroup_size(1)]]
fn main() {
    _ = o;
}

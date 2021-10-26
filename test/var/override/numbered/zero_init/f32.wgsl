[[override(1234)]] let o : f32 = f32();

[[stage(compute), workgroup_size(1)]]
fn main() {
    _ = o;
}

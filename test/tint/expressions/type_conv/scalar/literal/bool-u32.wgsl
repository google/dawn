var<private> u : u32 = u32(bool(true));

@compute @workgroup_size(1)
fn main() {
    _ = u;
}

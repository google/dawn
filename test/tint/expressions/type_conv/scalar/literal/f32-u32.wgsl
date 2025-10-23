var<private> u : u32 = u32(f32(1.0f));

@compute @workgroup_size(1)
fn main() {
    _ = u;
}

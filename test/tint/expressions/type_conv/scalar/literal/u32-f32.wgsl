var<private> u : f32 = f32(u32(1u));

@compute @workgroup_size(1)
fn main() {
    _ = u;
}

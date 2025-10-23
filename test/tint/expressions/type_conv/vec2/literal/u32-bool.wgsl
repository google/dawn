var<private> u : vec2<bool> = vec2<bool>(vec2<u32>(1u));

@compute @workgroup_size(1)
fn main() {
    _ = u;
}

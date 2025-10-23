var<private> u : vec2<u32> = vec2<u32>(vec2<bool>(true));

@compute @workgroup_size(1)
fn main() {
    _ = u;
}

var<private> u : vec2<i32> = vec2<i32>(vec2<bool>(true));

@compute @workgroup_size(1)
fn main() {
    _ = u;
}

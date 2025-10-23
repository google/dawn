var<private> u : vec2<bool> = vec2<bool>(vec2<i32>(1i));

@compute @workgroup_size(1)
fn main() {
  _ = u;
}

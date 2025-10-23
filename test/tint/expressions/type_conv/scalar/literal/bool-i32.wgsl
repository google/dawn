var<private> u : i32 = i32(bool(true));

@compute @workgroup_size(1)
fn main() {
    _ = u;
}

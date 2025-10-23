var<private> u : bool = bool(u32(1u));

@compute @workgroup_size(1)
fn main() {
    _ = u;
}

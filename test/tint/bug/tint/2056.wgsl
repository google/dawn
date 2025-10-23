fn f() -> f32 {
    return array(0.0, 0)[1];
}

@compute @workgroup_size(1)
fn main() {
    _ = f();
}

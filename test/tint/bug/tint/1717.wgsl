fn f() -> u32 {
    return 0x1u << 31u;
}

@compute @workgroup_size(1)
fn main() {
    _ = f();
}

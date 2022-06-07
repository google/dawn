override o : i32 = 1;

@compute @workgroup_size(1)
fn main() {
    _ = o;
}

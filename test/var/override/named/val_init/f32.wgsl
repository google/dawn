[[override]] let o : f32 = 1.0;

[[stage(compute), workgroup_size(1)]]
fn main() {
    _ = o;
}

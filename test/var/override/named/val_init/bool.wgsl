[[override]] let o : bool = true;

[[stage(compute), workgroup_size(1)]]
fn main() {
    _ = o;
}

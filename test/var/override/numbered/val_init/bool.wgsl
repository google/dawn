@id(1234) override o : bool = true;

@stage(compute) @workgroup_size(1)
fn main() {
    _ = o;
}

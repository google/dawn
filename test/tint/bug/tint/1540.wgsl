struct S {
    e: bool,
}

@stage(compute)
@workgroup_size(1)
fn main() {
    var b : bool;
    var v = S(true & b);
}

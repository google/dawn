struct S {
    i : i32,
    u : u32,
    f : f32,
    b : bool,
};

@compute @workgroup_size(1)
fn f() {
    var v = array<S, 4>();
}

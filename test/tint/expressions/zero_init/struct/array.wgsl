struct S {
    a : array<f32, 4>,
};

@compute @workgroup_size(1)
fn f() {
    var v = S();
}

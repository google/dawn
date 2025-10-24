var<private> arr = array<vec2<i32>, 2>(vec2<i32>(1i), vec2<i32>(2i));

@compute @workgroup_size(1)
fn f() {
    var v = arr;
}

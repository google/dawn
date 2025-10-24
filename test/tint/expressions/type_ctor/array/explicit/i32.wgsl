var<private> arr = array<i32, 2>(1i, 2i);

@compute @workgroup_size(1)
fn f() {
    var v = arr;
}

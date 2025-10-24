var<private> arr = array(mat2x2<f32>(1f, 2f, 3f, 4f),
                         mat2x2<f32>(5f, 6f, 7f, 8f));

@compute @workgroup_size(1)
fn f() {
    var v = arr;
}

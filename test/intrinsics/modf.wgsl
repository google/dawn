[[stage(compute), workgroup_size(1)]]
fn main() {
    var whole : f32;
    let frac : f32 = modf(1.23, &whole);
}

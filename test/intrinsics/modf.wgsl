[[stage(compute)]]
fn main() {
    var whole : f32;
    let frac : f32 = modf(1.23, &whole);
}

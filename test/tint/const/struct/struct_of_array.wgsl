@group(0) @binding(0) var<storage, read_write> s: array<u32>;

struct A {
    b: array<vec2u,2>,
}
@compute @workgroup_size(1u)
fn main(){
    const kStruct = A(array(vec2u(1,2), vec2u(3,4)));
    var q = 0u;
    s[0] = kStruct.b[q].x;
}

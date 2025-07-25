@group(0) @binding(0) var<storage, read_write> s: array<u32>;

@compute @workgroup_size(1u)
fn main(){
    const kArray = array(array(array(0u, 1u), array(2u, 3u)),
                    array(array(4u, 5u), array(6u, 7u)));
    var q = 0u;
    s[0] = kArray[q][q][q];
}

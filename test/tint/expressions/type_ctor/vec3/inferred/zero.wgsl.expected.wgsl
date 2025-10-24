var<private> f : vec3f = vec3();

var<private> i : vec3i = vec3();

var<private> u : vec3u = vec3();

@compute @workgroup_size(1)
fn main() {
  _ = f;
  _ = i;
  _ = u;
}

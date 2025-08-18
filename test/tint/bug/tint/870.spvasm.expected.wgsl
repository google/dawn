struct sspp962805860buildInformationS {
  footprint : vec4<f32>,
  offset : vec4<f32>,
  essence : i32,
  orientation : array<i32, 6u>,
}

struct S {
  passthru : sspp962805860buildInformationS,
}

@group(0u) @binding(2u) var<storage, read> sspp962805860buildInformation : S;

@fragment
fn main() {
  var orientation : array<i32, 6u>;
  let v = sspp962805860buildInformation.passthru.orientation;
  orientation[0i] = v[0u];
  orientation[1i] = v[1u];
  orientation[2i] = v[2u];
  orientation[3i] = v[3u];
  orientation[4i] = v[4u];
  orientation[5i] = v[5u];
}

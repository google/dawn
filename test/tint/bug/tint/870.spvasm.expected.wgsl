type Arr = array<i32, 6u>;

struct sspp962805860buildInformationS {
  footprint : vec4<f32>,
  offset : vec4<f32>,
  essence : i32,
  orientation : Arr,
}

struct x_B4_BuildInformation {
  passthru : sspp962805860buildInformationS,
}

@group(0) @binding(2) var<storage, read> sspp962805860buildInformation : x_B4_BuildInformation;

fn main_1() {
  var orientation : array<i32, 6u>;
  let x_23 : Arr = sspp962805860buildInformation.passthru.orientation;
  orientation[0i] = x_23[0u];
  orientation[1i] = x_23[1u];
  orientation[2i] = x_23[2u];
  orientation[3i] = x_23[3u];
  orientation[4i] = x_23[4u];
  orientation[5i] = x_23[5u];
  return;
}

@stage(fragment)
fn main() {
  main_1();
}

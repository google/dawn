SKIP: FAILED

#version 310 es
precision mediump float;

struct VertexInput {
  vec4 position;
  vec3 normal;
  vec4 tangent;
  vec2 texcoord;
  uvec4 joints;
  vec4 weights;
  vec4 instance0;
  vec4 instance1;
  vec4 instance2;
  vec4 instance3;
  vec4 instanceColor;
};
struct VertexOutput {
  vec4 position;
  vec3 worldPos;
  vec3 view;
  vec2 texcoord;
  vec2 texcoord2;
  vec4 color;
  vec4 instanceColor;
  vec3 normal;
  vec3 tangent;
  vec3 bitangent;
};
struct Camera {
  mat4 projection;
  mat4 inverseProjection;
  mat4 view;
  vec3 position;
  float time;
  vec2 outputSize;
  float zNear;
  float zFar;
};

layout (binding = 0) uniform Camera_1 {
  mat4 projection;
  mat4 inverseProjection;
  mat4 view;
  vec3 position;
  float time;
  vec2 outputSize;
  float zNear;
  float zFar;
} camera;

layout (binding = 1) buffer Joints_1 {
  mat4 matrices[];
} joint;
layout (binding = 2) buffer Joints_2 {
  mat4 matrices[];
} inverseBind;

mat4 getSkinMatrix(VertexInput tint_symbol) {
  mat4 joint0 = (joint.matrices[tint_symbol.joints.x] * inverseBind.matrices[tint_symbol.joints.x]);
  mat4 joint1 = (joint.matrices[tint_symbol.joints.y] * inverseBind.matrices[tint_symbol.joints.y]);
  mat4 joint2 = (joint.matrices[tint_symbol.joints.z] * inverseBind.matrices[tint_symbol.joints.z]);
  mat4 joint3 = (joint.matrices[tint_symbol.joints.w] * inverseBind.matrices[tint_symbol.joints.w]);
  mat4 skinMatrix = ((((joint0 * tint_symbol.weights.x) + (joint1 * tint_symbol.weights.y)) + (joint2 * tint_symbol.weights.z)) + (joint3 * tint_symbol.weights.w));
  return skinMatrix;
}

struct tint_symbol_3 {
  vec4 position;
  vec3 normal;
  vec4 tangent;
  vec2 texcoord;
  uvec4 joints;
  vec4 weights;
  vec4 instance0;
  vec4 instance1;
  vec4 instance2;
  vec4 instance3;
  vec4 instanceColor;
};
struct tint_symbol_4 {
  vec3 worldPos;
  vec3 view;
  vec2 texcoord;
  vec2 texcoord2;
  vec4 color;
  vec4 instanceColor;
  vec3 normal;
  vec3 tangent;
  vec3 bitangent;
  vec4 position;
};

VertexOutput vertexMain_inner(VertexInput tint_symbol) {
  VertexOutput tint_symbol_1 = VertexOutput(vec4(0.0f, 0.0f, 0.0f, 0.0f), vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 0.0f, 0.0f), vec2(0.0f, 0.0f), vec2(0.0f, 0.0f), vec4(0.0f, 0.0f, 0.0f, 0.0f), vec4(0.0f, 0.0f, 0.0f, 0.0f), vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 0.0f, 0.0f));
  mat4 modelMatrix = getSkinMatrix(tint_symbol);
  tint_symbol_1.normal = normalize((modelMatrix * vec4(tint_symbol.normal, 0.0f)).xyz);
  tint_symbol_1.tangent = normalize((modelMatrix * vec4(tint_symbol.tangent.xyz, 0.0f)).xyz);
  tint_symbol_1.bitangent = (cross(tint_symbol_1.normal, tint_symbol_1.tangent) * tint_symbol.tangent.w);
  tint_symbol_1.color = vec4(1.0f);
  tint_symbol_1.texcoord = tint_symbol.texcoord;
  tint_symbol_1.instanceColor = tint_symbol.instanceColor;
  vec4 modelPos = (modelMatrix * tint_symbol.position);
  tint_symbol_1.worldPos = modelPos.xyz;
  tint_symbol_1.view = (camera.position - modelPos.xyz);
  tint_symbol_1.position = ((camera.projection * camera.view) * modelPos);
  return tint_symbol_1;
}

tint_symbol_4 vertexMain(tint_symbol_3 tint_symbol_2) {
  VertexInput tint_symbol_5 = VertexInput(tint_symbol_2.position, tint_symbol_2.normal, tint_symbol_2.tangent, tint_symbol_2.texcoord, tint_symbol_2.joints, tint_symbol_2.weights, tint_symbol_2.instance0, tint_symbol_2.instance1, tint_symbol_2.instance2, tint_symbol_2.instance3, tint_symbol_2.instanceColor);
  VertexOutput inner_result = vertexMain_inner(tint_symbol_5);
  tint_symbol_4 wrapper_result = tint_symbol_4(vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 0.0f, 0.0f), vec2(0.0f, 0.0f), vec2(0.0f, 0.0f), vec4(0.0f, 0.0f, 0.0f, 0.0f), vec4(0.0f, 0.0f, 0.0f, 0.0f), vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 0.0f, 0.0f), vec4(0.0f, 0.0f, 0.0f, 0.0f));
  wrapper_result.position = inner_result.position;
  wrapper_result.worldPos = inner_result.worldPos;
  wrapper_result.view = inner_result.view;
  wrapper_result.texcoord = inner_result.texcoord;
  wrapper_result.texcoord2 = inner_result.texcoord2;
  wrapper_result.color = inner_result.color;
  wrapper_result.instanceColor = inner_result.instanceColor;
  wrapper_result.normal = inner_result.normal;
  wrapper_result.tangent = inner_result.tangent;
  wrapper_result.bitangent = inner_result.bitangent;
  return wrapper_result;
}
in vec4 position;
in vec3 normal;
in vec4 tangent;
in vec2 texcoord;
in uvec4 joints;
in vec4 weights;
in vec4 instance0;
in vec4 instance1;
in vec4 instance2;
in vec4 instance3;
in vec4 instanceColor;
out vec3 worldPos;
out vec3 view;
out vec2 texcoord;
out vec2 texcoord2;
out vec4 color;
out vec4 instanceColor;
out vec3 normal;
out vec3 tangent;
out vec3 bitangent;
void main() {
  tint_symbol_3 inputs;
  inputs.position = position;
  inputs.normal = normal;
  inputs.tangent = tangent;
  inputs.texcoord = texcoord;
  inputs.joints = joints;
  inputs.weights = weights;
  inputs.instance0 = instance0;
  inputs.instance1 = instance1;
  inputs.instance2 = instance2;
  inputs.instance3 = instance3;
  inputs.instanceColor = instanceColor;
  tint_symbol_4 outputs;
  outputs = vertexMain(inputs);
  worldPos = outputs.worldPos;
  view = outputs.view;
  texcoord = outputs.texcoord;
  texcoord2 = outputs.texcoord2;
  color = outputs.color;
  instanceColor = outputs.instanceColor;
  normal = outputs.normal;
  tangent = outputs.tangent;
  bitangent = outputs.bitangent;
  gl_Position = outputs.position;
  gl_Position.y = -gl_Position.y;
}


Error parsing GLSL shader:
ERROR: 0:138: 'texcoord' : redefinition 
ERROR: 1 compilation errors.  No code generated.




SKIP: FAILED

#version 310 es

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

struct Joints {
  mat4 matrices[];
};

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

uniform Camera camera;
Joints joint;
Joints inverseBind;
mat4 getInstanceMatrix(VertexInput tint_symbol) {
  return mat4(tint_symbol.instance0, tint_symbol.instance1, tint_symbol.instance2, tint_symbol.instance3);
}
mat4 getSkinMatrix(VertexInput tint_symbol) {
  mat4 joint0 = (joint.matrices[tint_symbol.joints[0u]] * inverseBind.matrices[tint_symbol.joints[0u]]);
  mat4 joint1 = (joint.matrices[tint_symbol.joints[1u]] * inverseBind.matrices[tint_symbol.joints[1u]]);
  mat4 joint2 = (joint.matrices[tint_symbol.joints[2u]] * inverseBind.matrices[tint_symbol.joints[2u]]);
  mat4 joint3 = (joint.matrices[tint_symbol.joints[3u]] * inverseBind.matrices[tint_symbol.joints[3u]]);
  mat4 skinMatrix = ((((joint0 * tint_symbol.weights[0u]) + (joint1 * tint_symbol.weights[1u])) + (joint2 * tint_symbol.weights[2u])) + (joint3 * tint_symbol.weights[3u]));
  return skinMatrix;
}
VertexOutput main(VertexInput tint_symbol) {
  VertexOutput tint_symbol_1 = VertexOutput(vec4(0.0f), vec3(0.0f), vec3(0.0f), vec2(0.0f), vec2(0.0f), vec4(0.0f), vec4(0.0f), vec3(0.0f), vec3(0.0f), vec3(0.0f));
  mat4 modelMatrix = getSkinMatrix(tint_symbol);
  tint_symbol_1.normal = normalize((modelMatrix * vec4(tint_symbol.normal, 0.0f)).xyz);
  tint_symbol_1.tangent = normalize((modelMatrix * vec4(tint_symbol.tangent.xyz, 0.0f)).xyz);
  tint_symbol_1.bitangent = (cross(tint_symbol_1.normal, tint_symbol_1.tangent) * tint_symbol.tangent[3u]);
  tint_symbol_1.color = vec4(1.0f);
  tint_symbol_1.texcoord = tint_symbol.texcoord;
  tint_symbol_1.instanceColor = tint_symbol.instanceColor;
  vec4 modelPos = (modelMatrix * tint_symbol.position);
  tint_symbol_1.worldPos = modelPos.xyz;
  tint_symbol_1.view = (camera.position - modelPos.xyz);
  tint_symbol_1.position = ((camera.projection * camera.view) * modelPos);
  return tint_symbol_1;
}
error: Error parsing GLSL shader:
ERROR: 0:15: '' : array size required 
ERROR: 0:16: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1

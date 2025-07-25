struct main_inputs {
  float none : TEXCOORD0;
  nointerpolation float flat : TEXCOORD1;
  linear float perspective_center : TEXCOORD2;
  linear centroid float perspective_centroid : TEXCOORD3;
  linear sample float perspective_sample : TEXCOORD4;
  noperspective float linear_center : TEXCOORD5;
  noperspective centroid float linear_centroid : TEXCOORD6;
  noperspective sample float linear_sample : TEXCOORD7;
  linear float perspective_default : TEXCOORD8;
  noperspective float linear_default : TEXCOORD9;
};


void main_inner(float none, float flat, float perspective_center, float perspective_centroid, float perspective_sample, float linear_center, float linear_centroid, float linear_sample, float perspective_default, float linear_default) {
}

void main(main_inputs inputs) {
  main_inner(inputs.none, inputs.flat, inputs.perspective_center, inputs.perspective_centroid, inputs.perspective_sample, inputs.linear_center, inputs.linear_centroid, inputs.linear_sample, inputs.perspective_default, inputs.linear_default);
}


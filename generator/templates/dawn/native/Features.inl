// Copyright 2023 The Dawn Authors
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

namespace dawn::native {

wgpu::FeatureName ToAPI(Feature feature) {
  switch (feature) {
    {% for enum in types["feature name"].values if enum.valid %}
      case Feature::{{as_cppEnum(enum.name)}}:
        return wgpu::FeatureName::{{as_cppEnum(enum.name)}};
    {% endfor %}
    case Feature::InvalidEnum:
      UNREACHABLE();
  }
}

Feature FromAPI(wgpu::FeatureName feature) {
  switch (feature) {
    {% for enum in types["feature name"].values %}
      case wgpu::FeatureName::{{as_cppEnum(enum.name)}}:
        {% if enum.valid %}
          return Feature::{{as_cppEnum(enum.name)}};
        {% else %}
          return Feature::InvalidEnum;
        {% endif %}
    {% endfor %}
    default:
      return Feature::InvalidEnum;
  }
}

static constexpr bool FeatureInfoIsDefined(Feature feature) {
  for (const auto& info : kFeatureInfo) {
    if (info.feature == feature) {
      return true;
    }
  }
  return false;
}

static constexpr ityp::array<Feature, FeatureInfo, kEnumCount<Feature>> InitializeFeatureEnumAndInfoList() {
  constexpr size_t kInfoCount = sizeof(kFeatureInfo) / sizeof(kFeatureInfo[0]);
  ityp::array<Feature, FeatureInfo, kEnumCount<Feature>> list{};
  {% for enum in types["feature name"].values if enum.valid %}
    {
      static_assert(FeatureInfoIsDefined(Feature::{{as_cppEnum(enum.name)}}),
                    "Please define feature info for {{as_cppEnum(enum.name)}} in Features.cpp");
      for (size_t i = 0; i < kInfoCount; ++i) {
        if (kFeatureInfo[i].feature == Feature::{{as_cppEnum(enum.name)}}) {
          list[Feature::{{as_cppEnum(enum.name)}}] = {
            "{{enum.name.snake_case()}}",
            kFeatureInfo[i].info.description,
            kFeatureInfo[i].info.url,
            kFeatureInfo[i].info.featureState,
          };
        }
      }
    }
  {% endfor %}
  return list;
}

const ityp::array<Feature, FeatureInfo, kEnumCount<Feature>> kFeatureNameAndInfoList = InitializeFeatureEnumAndInfoList();

}

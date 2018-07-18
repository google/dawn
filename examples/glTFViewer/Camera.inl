// Copyright 2017 The Dawn Authors
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

class Camera {
    public:
        Camera()
            : _azimuth(glm::radians(45.f)),
              _altitude(glm::radians(30.f)),
              _radius(10.f),
              _center(0, 0, 0),
              _dirty(true) {
            recalculate();
        }

        void rotate(float dAzimuth, float dAltitude) {
            _dirty = true;
            _azimuth = glm::mod(_azimuth + dAzimuth, glm::radians(360.f));
            _altitude = glm::clamp(_altitude + dAltitude, glm::radians(-89.f), glm::radians(89.f));
        }

        void pan(float dX, float dY) {
            recalculate();
            glm::vec3 vX = glm::normalize(glm::cross(-_eyeDir, glm::vec3(0, 1, 0)));
            glm::vec3 vY = glm::normalize(glm::cross(_eyeDir, vX));
            _center += vX * dX * _radius + vY * dY * _radius;
        }

        void zoom(float factor) {
            _dirty = true;
            _radius = _radius * glm::exp(-factor);
        }

        glm::mat4 view() {
            if (_dirty) {
                recalculate();
            }
            return _view;
        }
    private:
        void recalculate() {
            glm::vec4 eye4 = glm::vec4(1, 0, 0, 1);
            eye4 = glm::rotate(glm::mat4(), _altitude, glm::vec3(0, 0, 1)) * eye4;
            eye4 = glm::rotate(glm::mat4(), _azimuth, glm::vec3(0, 1, 0)) * eye4;
            _eyeDir = glm::vec3(eye4);

            _view = glm::lookAt(_center + _eyeDir * _radius, _center, glm::vec3(0, -1, 0));
            _dirty = false;
        }
        float _azimuth;
        float _altitude;
        float _radius;
        glm::vec3 _center;
        glm::vec3 _eyeDir;
        bool _dirty;
        glm::mat4 _view;
};

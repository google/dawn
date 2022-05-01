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

#include "dawn/tests/DawnTest.h"

#include <algorithm>
#include <fstream>
#include <iomanip>
#include <regex>
#include <set>
#include <sstream>
#include <unordered_map>
#include <unordered_set>

#include "dawn/common/Assert.h"
#include "dawn/common/GPUInfo.h"
#include "dawn/common/Log.h"
#include "dawn/common/Math.h"
#include "dawn/common/Platform.h"
#include "dawn/common/SystemUtils.h"
#include "dawn/dawn_proc.h"
#include "dawn/native/Device.h"
#include "dawn/native/Instance.h"
#include "dawn/native/dawn_platform.h"
#include "dawn/utils/ComboRenderPipelineDescriptor.h"
#include "dawn/utils/PlatformDebugLogger.h"
#include "dawn/utils/SystemUtils.h"
#include "dawn/utils/TerribleCommandBuffer.h"
#include "dawn/utils/TestUtils.h"
#include "dawn/utils/WGPUHelpers.h"
#include "dawn/utils/WireHelper.h"
#include "dawn/wire/WireClient.h"
#include "dawn/wire/WireServer.h"

#if defined(DAWN_ENABLE_BACKEND_OPENGL)
#include "GLFW/glfw3.h"
#include "dawn/native/OpenGLBackend.h"
#endif  // DAWN_ENABLE_BACKEND_OPENGL

namespace {

std::string ParamName(wgpu::BackendType type) {
    switch (type) {
        case wgpu::BackendType::D3D12:
            return "D3D12";
        case wgpu::BackendType::Metal:
            return "Metal";
        case wgpu::BackendType::Null:
            return "Null";
        case wgpu::BackendType::OpenGL:
            return "OpenGL";
        case wgpu::BackendType::OpenGLES:
            return "OpenGLES";
        case wgpu::BackendType::Vulkan:
            return "Vulkan";
        default:
            UNREACHABLE();
    }
}

const char* AdapterTypeName(wgpu::AdapterType type) {
    switch (type) {
        case wgpu::AdapterType::DiscreteGPU:
            return "Discrete GPU";
        case wgpu::AdapterType::IntegratedGPU:
            return "Integrated GPU";
        case wgpu::AdapterType::CPU:
            return "CPU";
        case wgpu::AdapterType::Unknown:
            return "Unknown";
        default:
            UNREACHABLE();
    }
}

struct MapReadUserdata {
    DawnTestBase* test;
    size_t slot;
};

DawnTestEnvironment* gTestEnv = nullptr;

template <typename T>
void printBuffer(testing::AssertionResult& result, const T* buffer, const size_t count) {
    static constexpr unsigned int kBytes = sizeof(T);

    for (size_t index = 0; index < count; ++index) {
        auto byteView = reinterpret_cast<const uint8_t*>(buffer + index);
        for (unsigned int b = 0; b < kBytes; ++b) {
            char buf[4];
            snprintf(buf, sizeof(buf), "%02X ", byteView[b]);
            result << buf;
        }
    }
    result << std::endl;
}

}  // anonymous namespace

const RGBA8 RGBA8::kZero = RGBA8(0, 0, 0, 0);
const RGBA8 RGBA8::kBlack = RGBA8(0, 0, 0, 255);
const RGBA8 RGBA8::kRed = RGBA8(255, 0, 0, 255);
const RGBA8 RGBA8::kGreen = RGBA8(0, 255, 0, 255);
const RGBA8 RGBA8::kBlue = RGBA8(0, 0, 255, 255);
const RGBA8 RGBA8::kYellow = RGBA8(255, 255, 0, 255);
const RGBA8 RGBA8::kWhite = RGBA8(255, 255, 255, 255);

BackendTestConfig::BackendTestConfig(wgpu::BackendType backendType,
                                     std::initializer_list<const char*> forceEnabledWorkarounds,
                                     std::initializer_list<const char*> forceDisabledWorkarounds)
    : backendType(backendType),
      forceEnabledWorkarounds(forceEnabledWorkarounds),
      forceDisabledWorkarounds(forceDisabledWorkarounds) {}

BackendTestConfig D3D12Backend(std::initializer_list<const char*> forceEnabledWorkarounds,
                               std::initializer_list<const char*> forceDisabledWorkarounds) {
    return BackendTestConfig(wgpu::BackendType::D3D12, forceEnabledWorkarounds,
                             forceDisabledWorkarounds);
}

BackendTestConfig MetalBackend(std::initializer_list<const char*> forceEnabledWorkarounds,
                               std::initializer_list<const char*> forceDisabledWorkarounds) {
    return BackendTestConfig(wgpu::BackendType::Metal, forceEnabledWorkarounds,
                             forceDisabledWorkarounds);
}

BackendTestConfig NullBackend(std::initializer_list<const char*> forceEnabledWorkarounds,
                              std::initializer_list<const char*> forceDisabledWorkarounds) {
    return BackendTestConfig(wgpu::BackendType::Null, forceEnabledWorkarounds,
                             forceDisabledWorkarounds);
}

BackendTestConfig OpenGLBackend(std::initializer_list<const char*> forceEnabledWorkarounds,
                                std::initializer_list<const char*> forceDisabledWorkarounds) {
    return BackendTestConfig(wgpu::BackendType::OpenGL, forceEnabledWorkarounds,
                             forceDisabledWorkarounds);
}

BackendTestConfig OpenGLESBackend(std::initializer_list<const char*> forceEnabledWorkarounds,
                                  std::initializer_list<const char*> forceDisabledWorkarounds) {
    return BackendTestConfig(wgpu::BackendType::OpenGLES, forceEnabledWorkarounds,
                             forceDisabledWorkarounds);
}

BackendTestConfig VulkanBackend(std::initializer_list<const char*> forceEnabledWorkarounds,
                                std::initializer_list<const char*> forceDisabledWorkarounds) {
    return BackendTestConfig(wgpu::BackendType::Vulkan, forceEnabledWorkarounds,
                             forceDisabledWorkarounds);
}

TestAdapterProperties::TestAdapterProperties(const wgpu::AdapterProperties& properties,
                                             bool selected)
    : wgpu::AdapterProperties(properties), adapterName(properties.name), selected(selected) {}

AdapterTestParam::AdapterTestParam(const BackendTestConfig& config,
                                   const TestAdapterProperties& adapterProperties)
    : adapterProperties(adapterProperties),
      forceEnabledWorkarounds(config.forceEnabledWorkarounds),
      forceDisabledWorkarounds(config.forceDisabledWorkarounds) {}

std::ostream& operator<<(std::ostream& os, const AdapterTestParam& param) {
    os << ParamName(param.adapterProperties.backendType) << " "
       << param.adapterProperties.adapterName;

    // In a Windows Remote Desktop session there are two adapters named "Microsoft Basic Render
    // Driver" with different adapter types. We must differentiate them to avoid any tests using the
    // same name.
    if (param.adapterProperties.deviceID == 0x008C) {
        std::string adapterType = AdapterTypeName(param.adapterProperties.adapterType);
        os << " " << adapterType;
    }

    for (const char* forceEnabledWorkaround : param.forceEnabledWorkarounds) {
        os << "; e:" << forceEnabledWorkaround;
    }
    for (const char* forceDisabledWorkaround : param.forceDisabledWorkarounds) {
        os << "; d:" << forceDisabledWorkaround;
    }
    return os;
}

DawnTestBase::PrintToStringParamName::PrintToStringParamName(const char* test) : mTest(test) {}

std::string DawnTestBase::PrintToStringParamName::SanitizeParamName(std::string paramName,
                                                                    size_t index) const {
    // Sanitize the adapter name for GoogleTest
    std::string sanitizedName = std::regex_replace(paramName, std::regex("[^a-zA-Z0-9]+"), "_");

    // Strip trailing underscores, if any.
    while (sanitizedName.back() == '_') {
        sanitizedName.resize(sanitizedName.length() - 1);
    }

    // We don't know the the test name at this point, but the format usually looks like
    // this.
    std::string prefix = mTest + ".TheTestNameUsuallyGoesHere/";
    std::string testFormat = prefix + sanitizedName;
    if (testFormat.length() > 220) {
        // The bots don't support test names longer than 256. Shorten the name and append a unique
        // index if we're close. The failure log will still print the full param name.
        std::string suffix = std::string("__") + std::to_string(index);
        size_t targetLength = sanitizedName.length();
        targetLength -= testFormat.length() - 220;
        targetLength -= suffix.length();
        sanitizedName.resize(targetLength);
        sanitizedName = sanitizedName + suffix;
    }
    return sanitizedName;
}

// Implementation of DawnTestEnvironment

void InitDawnEnd2EndTestEnvironment(int argc, char** argv) {
    gTestEnv = new DawnTestEnvironment(argc, argv);
    testing::AddGlobalTestEnvironment(gTestEnv);
}

// static
void DawnTestEnvironment::SetEnvironment(DawnTestEnvironment* env) {
    gTestEnv = env;
}

DawnTestEnvironment::DawnTestEnvironment(int argc, char** argv) {
    ParseArgs(argc, argv);

    if (mBackendValidationLevel != dawn::native::BackendValidationLevel::Disabled) {
        mPlatformDebugLogger =
            std::unique_ptr<utils::PlatformDebugLogger>(utils::CreatePlatformDebugLogger());
    }

    // Create a temporary instance to select available and preferred adapters. This is done before
    // test instantiation so GetAvailableAdapterTestParamsForBackends can generate test
    // parameterizations all selected adapters. We drop the instance at the end of this function
    // because the Vulkan validation layers use static global mutexes which behave badly when
    // Chromium's test launcher forks the test process. The instance will be recreated on test
    // environment setup.
    std::unique_ptr<dawn::native::Instance> instance = CreateInstanceAndDiscoverAdapters();
    ASSERT(instance);

    SelectPreferredAdapterProperties(instance.get());
    PrintTestConfigurationAndAdapterInfo(instance.get());
}

DawnTestEnvironment::~DawnTestEnvironment() = default;

void DawnTestEnvironment::ParseArgs(int argc, char** argv) {
    size_t argLen = 0;  // Set when parsing --arg=X arguments
    for (int i = 1; i < argc; ++i) {
        if (strcmp("-w", argv[i]) == 0 || strcmp("--use-wire", argv[i]) == 0) {
            mUseWire = true;
            continue;
        }

        if (strcmp("--run-suppressed-tests", argv[i]) == 0) {
            mRunSuppressedTests = true;
            continue;
        }

        constexpr const char kEnableBackendValidationSwitch[] = "--enable-backend-validation";
        argLen = sizeof(kEnableBackendValidationSwitch) - 1;
        if (strncmp(argv[i], kEnableBackendValidationSwitch, argLen) == 0) {
            const char* level = argv[i] + argLen;
            if (level[0] != '\0') {
                if (strcmp(level, "=full") == 0) {
                    mBackendValidationLevel = dawn::native::BackendValidationLevel::Full;
                } else if (strcmp(level, "=partial") == 0) {
                    mBackendValidationLevel = dawn::native::BackendValidationLevel::Partial;
                } else if (strcmp(level, "=disabled") == 0) {
                    mBackendValidationLevel = dawn::native::BackendValidationLevel::Disabled;
                } else {
                    dawn::ErrorLog() << "Invalid backend validation level" << level;
                    UNREACHABLE();
                }
            } else {
                mBackendValidationLevel = dawn::native::BackendValidationLevel::Partial;
            }
            continue;
        }

        if (strcmp("-c", argv[i]) == 0 || strcmp("--begin-capture-on-startup", argv[i]) == 0) {
            mBeginCaptureOnStartup = true;
            continue;
        }

        if (mToggleParser.ParseEnabledToggles(argv[i])) {
            continue;
        }

        if (mToggleParser.ParseDisabledToggles(argv[i])) {
            continue;
        }

        constexpr const char kVendorIdFilterArg[] = "--adapter-vendor-id=";
        argLen = sizeof(kVendorIdFilterArg) - 1;
        if (strncmp(argv[i], kVendorIdFilterArg, argLen) == 0) {
            const char* vendorIdFilter = argv[i] + argLen;
            if (vendorIdFilter[0] != '\0') {
                mVendorIdFilter = strtoul(vendorIdFilter, nullptr, 16);
                // Set filter flag if vendor id is non-zero.
                mHasVendorIdFilter = mVendorIdFilter != 0;
            }
            continue;
        }

        constexpr const char kExclusiveDeviceTypePreferenceArg[] =
            "--exclusive-device-type-preference=";
        argLen = sizeof(kExclusiveDeviceTypePreferenceArg) - 1;
        if (strncmp(argv[i], kExclusiveDeviceTypePreferenceArg, argLen) == 0) {
            const char* preference = argv[i] + argLen;
            if (preference[0] != '\0') {
                std::istringstream ss(preference);
                std::string type;
                while (std::getline(ss, type, ',')) {
                    if (strcmp(type.c_str(), "discrete") == 0) {
                        mDevicePreferences.push_back(wgpu::AdapterType::DiscreteGPU);
                    } else if (strcmp(type.c_str(), "integrated") == 0) {
                        mDevicePreferences.push_back(wgpu::AdapterType::IntegratedGPU);
                    } else if (strcmp(type.c_str(), "cpu") == 0) {
                        mDevicePreferences.push_back(wgpu::AdapterType::CPU);
                    } else {
                        dawn::ErrorLog() << "Invalid device type preference: " << type;
                        UNREACHABLE();
                    }
                }
            }
            continue;
        }

        constexpr const char kWireTraceDirArg[] = "--wire-trace-dir=";
        argLen = sizeof(kWireTraceDirArg) - 1;
        if (strncmp(argv[i], kWireTraceDirArg, argLen) == 0) {
            mWireTraceDir = argv[i] + argLen;
            continue;
        }

        constexpr const char kBackendArg[] = "--backend=";
        argLen = sizeof(kBackendArg) - 1;
        if (strncmp(argv[i], kBackendArg, argLen) == 0) {
            const char* param = argv[i] + argLen;
            if (strcmp("d3d12", param) == 0) {
                mBackendTypeFilter = wgpu::BackendType::D3D12;
            } else if (strcmp("metal", param) == 0) {
                mBackendTypeFilter = wgpu::BackendType::Metal;
            } else if (strcmp("null", param) == 0) {
                mBackendTypeFilter = wgpu::BackendType::Null;
            } else if (strcmp("opengl", param) == 0) {
                mBackendTypeFilter = wgpu::BackendType::OpenGL;
            } else if (strcmp("opengles", param) == 0) {
                mBackendTypeFilter = wgpu::BackendType::OpenGLES;
            } else if (strcmp("vulkan", param) == 0) {
                mBackendTypeFilter = wgpu::BackendType::Vulkan;
            } else {
                dawn::ErrorLog()
                    << "Invalid backend \"" << param
                    << "\". Valid backends are: d3d12, metal, null, opengl, opengles, vulkan.";
                UNREACHABLE();
            }
            mHasBackendTypeFilter = true;
            continue;
        }
        if (strcmp("-h", argv[i]) == 0 || strcmp("--help", argv[i]) == 0) {
            dawn::InfoLog()
                << "\n\nUsage: " << argv[0]
                << " [GTEST_FLAGS...] [-w] [-c]\n"
                   "    [--enable-toggles=toggles] [--disable-toggles=toggles]\n"
                   "    [--backend=x]\n"
                   "    [--adapter-vendor-id=x] "
                   "[--enable-backend-validation[=full,partial,disabled]]\n"
                   "    [--exclusive-device-type-preference=integrated,cpu,discrete]\n\n"
                   "  -w, --use-wire: Run the tests through the wire (defaults to no wire)\n"
                   "  -c, --begin-capture-on-startup: Begin debug capture on startup "
                   "(defaults to no capture)\n"
                   "  --enable-backend-validation: Enables backend validation. Defaults to \n"
                   "    'partial' to enable only minimum backend validation. Set to 'full' to\n"
                   "    enable all available backend validation with less performance overhead.\n"
                   "    Set to 'disabled' to run with no validation (same as no flag).\n"
                   "  --enable-toggles: Comma-delimited list of Dawn toggles to enable.\n"
                   "    ex.) skip_validation,disable_robustness,turn_off_vsync\n"
                   "  --disable-toggles: Comma-delimited list of Dawn toggles to disable\n"
                   "  --adapter-vendor-id: Select adapter by vendor id to run end2end tests"
                   "on multi-GPU systems \n"
                   "  --backend: Select adapter by backend type. Valid backends are: d3d12, metal, "
                   "null, opengl, opengles, vulkan\n"
                   "  --exclusive-device-type-preference: Comma-delimited list of preferred device "
                   "types. For each backend, tests will run only on adapters that match the first "
                   "available device type\n"
                   "  --run-suppressed-tests: Run all the tests that will be skipped by the macro "
                   "DAWN_SUPPRESS_TEST_IF()\n";
            continue;
        }

        // Skip over args that look like they're for Googletest.
        constexpr const char kGtestArgPrefix[] = "--gtest_";
        if (strncmp(kGtestArgPrefix, argv[i], sizeof(kGtestArgPrefix) - 1) == 0) {
            continue;
        }

        dawn::WarningLog() << " Unused argument: " << argv[i];
    }
}

std::unique_ptr<dawn::native::Instance> DawnTestEnvironment::CreateInstanceAndDiscoverAdapters() {
    auto instance = std::make_unique<dawn::native::Instance>();
    instance->EnableBeginCaptureOnStartup(mBeginCaptureOnStartup);
    instance->SetBackendValidationLevel(mBackendValidationLevel);
    instance->DiscoverDefaultAdapters();

#ifdef DAWN_ENABLE_BACKEND_DESKTOP_GL
    if (!glfwInit()) {
        return instance;
    }
    glfwDefaultWindowHints();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);

    mOpenGLWindow = glfwCreateWindow(400, 400, "Dawn OpenGL test window", nullptr, nullptr);
    if (mOpenGLWindow != nullptr) {
        glfwMakeContextCurrent(mOpenGLWindow);
        dawn::native::opengl::AdapterDiscoveryOptions adapterOptions;
        adapterOptions.getProc = reinterpret_cast<void* (*)(const char*)>(glfwGetProcAddress);
        instance->DiscoverAdapters(&adapterOptions);
    }
#endif  // DAWN_ENABLE_BACKEND_DESKTOP_GL

#ifdef DAWN_ENABLE_BACKEND_OPENGLES

    ScopedEnvironmentVar angleDefaultPlatform;
    if (GetEnvironmentVar("ANGLE_DEFAULT_PLATFORM").first.empty()) {
        angleDefaultPlatform.Set("ANGLE_DEFAULT_PLATFORM", "swiftshader");
    }

    if (!glfwInit()) {
        return instance;
    }
    glfwDefaultWindowHints();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
    glfwWindowHint(GLFW_CONTEXT_CREATION_API, GLFW_EGL_CONTEXT_API);
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);

    mOpenGLESWindow = glfwCreateWindow(400, 400, "Dawn OpenGLES test window", nullptr, nullptr);
    if (mOpenGLESWindow != nullptr) {
        glfwMakeContextCurrent(mOpenGLESWindow);
        dawn::native::opengl::AdapterDiscoveryOptionsES adapterOptionsES;
        adapterOptionsES.getProc = reinterpret_cast<void* (*)(const char*)>(glfwGetProcAddress);
        instance->DiscoverAdapters(&adapterOptionsES);
        glfwWindowHint(GLFW_VISIBLE, GLFW_TRUE);
    }
#endif  // DAWN_ENABLE_BACKEND_OPENGLES

    return instance;
}

GLFWwindow* DawnTestEnvironment::GetOpenGLWindow() const {
    return mOpenGLWindow;
}

GLFWwindow* DawnTestEnvironment::GetOpenGLESWindow() const {
    return mOpenGLESWindow;
}

void DawnTestEnvironment::SelectPreferredAdapterProperties(const dawn::native::Instance* instance) {
    // Get the first available preferred device type.
    wgpu::AdapterType preferredDeviceType = static_cast<wgpu::AdapterType>(-1);
    bool hasDevicePreference = false;
    for (wgpu::AdapterType devicePreference : mDevicePreferences) {
        for (const dawn::native::Adapter& adapter : instance->GetAdapters()) {
            wgpu::AdapterProperties properties;
            adapter.GetProperties(&properties);

            if (properties.adapterType == devicePreference) {
                preferredDeviceType = devicePreference;
                hasDevicePreference = true;
                break;
            }
        }
        if (hasDevicePreference) {
            break;
        }
    }

    std::set<std::pair<wgpu::BackendType, std::string>> adapterNameSet;
    for (const dawn::native::Adapter& adapter : instance->GetAdapters()) {
        wgpu::AdapterProperties properties;
        adapter.GetProperties(&properties);

        // All adapters are selected by default.
        bool selected = true;
        // The adapter is deselected if:
        if (mHasBackendTypeFilter) {
            // It doesn't match the backend type, if present.
            selected &= properties.backendType == mBackendTypeFilter;
        }
        if (mHasVendorIdFilter) {
            // It doesn't match the vendor id, if present.
            selected &= mVendorIdFilter == properties.vendorID;

            if (!mDevicePreferences.empty()) {
                dawn::WarningLog() << "Vendor ID filter provided. Ignoring device type preference.";
            }
        }
        if (hasDevicePreference) {
            // There is a device preference and:
            selected &=
                // The device type doesn't match the first available preferred type for that
                // backend, if present.
                (properties.adapterType == preferredDeviceType) ||
                // Always select Unknown OpenGL adapters if we don't want a CPU adapter.
                // OpenGL will usually be unknown because we can't query the device type.
                // If we ever have Swiftshader GL (unlikely), we could set the DeviceType properly.
                (preferredDeviceType != wgpu::AdapterType::CPU &&
                 properties.adapterType == wgpu::AdapterType::Unknown &&
                 (properties.backendType == wgpu::BackendType::OpenGL ||
                  properties.backendType == wgpu::BackendType::OpenGLES)) ||
                // Always select the Null backend. There are few tests on this backend, and they run
                // quickly. This is temporary as to not lose coverage. We can group it with
                // Swiftshader as a CPU adapter when we have Swiftshader tests.
                (properties.backendType == wgpu::BackendType::Null);
        }

        // In Windows Remote Desktop sessions we may be able to discover multiple adapters that
        // have the same name and backend type. We will just choose one adapter from them in our
        // tests.
        const auto adapterTypeAndName =
            std::make_pair(properties.backendType, std::string(properties.name));
        if (adapterNameSet.find(adapterTypeAndName) == adapterNameSet.end()) {
            adapterNameSet.insert(adapterTypeAndName);
            mAdapterProperties.emplace_back(properties, selected);
        }
    }
}

std::vector<AdapterTestParam> DawnTestEnvironment::GetAvailableAdapterTestParamsForBackends(
    const BackendTestConfig* params,
    size_t numParams) {
    std::vector<AdapterTestParam> testParams;
    for (size_t i = 0; i < numParams; ++i) {
        for (const auto& adapterProperties : mAdapterProperties) {
            if (params[i].backendType == adapterProperties.backendType &&
                adapterProperties.selected) {
                testParams.push_back(AdapterTestParam(params[i], adapterProperties));
            }
        }
    }
    return testParams;
}

void DawnTestEnvironment::PrintTestConfigurationAndAdapterInfo(
    dawn::native::Instance* instance) const {
    dawn::LogMessage log = dawn::InfoLog();
    log << "Testing configuration\n"
           "---------------------\n"
           "UseWire: "
        << (mUseWire ? "true" : "false")
        << "\n"
           "Run suppressed tests: "
        << (mRunSuppressedTests ? "true" : "false")
        << "\n"
           "BackendValidation: ";

    switch (mBackendValidationLevel) {
        case dawn::native::BackendValidationLevel::Full:
            log << "full";
            break;
        case dawn::native::BackendValidationLevel::Partial:
            log << "partial";
            break;
        case dawn::native::BackendValidationLevel::Disabled:
            log << "disabled";
            break;
        default:
            UNREACHABLE();
    }

    if (GetEnabledToggles().size() > 0) {
        log << "\n"
               "Enabled Toggles\n";
        for (const std::string& toggle : GetEnabledToggles()) {
            const dawn::native::ToggleInfo* info = instance->GetToggleInfo(toggle.c_str());
            ASSERT(info != nullptr);
            log << " - " << info->name << ": " << info->description << "\n";
        }
    }

    if (GetDisabledToggles().size() > 0) {
        log << "\n"
               "Disabled Toggles\n";
        for (const std::string& toggle : GetDisabledToggles()) {
            const dawn::native::ToggleInfo* info = instance->GetToggleInfo(toggle.c_str());
            ASSERT(info != nullptr);
            log << " - " << info->name << ": " << info->description << "\n";
        }
    }

    log << "\n"
           "BeginCaptureOnStartup: "
        << (mBeginCaptureOnStartup ? "true" : "false")
        << "\n"
           "\n"
        << "System adapters: \n";

    for (const TestAdapterProperties& properties : mAdapterProperties) {
        std::ostringstream vendorId;
        std::ostringstream deviceId;
        vendorId << std::setfill('0') << std::uppercase << std::internal << std::hex << std::setw(4)
                 << properties.vendorID;
        deviceId << std::setfill('0') << std::uppercase << std::internal << std::hex << std::setw(4)
                 << properties.deviceID;

        // Preparing for outputting hex numbers
        log << std::showbase << std::hex << std::setfill('0') << std::setw(4)

            << " - \"" << properties.adapterName << "\" - \"" << properties.driverDescription
            << "\"\n"
            << "   type: " << AdapterTypeName(properties.adapterType)
            << ", backend: " << ParamName(properties.backendType) << "\n"
            << "   vendorId: 0x" << vendorId.str() << ", deviceId: 0x" << deviceId.str()
            << (properties.selected ? " [Selected]" : "") << "\n";
    }
}

void DawnTestEnvironment::SetUp() {
    mInstance = CreateInstanceAndDiscoverAdapters();
    ASSERT(mInstance);
}

void DawnTestEnvironment::TearDown() {
    // When Vulkan validation layers are enabled, it's unsafe to call Vulkan APIs in the destructor
    // of a static/global variable, so the instance must be manually released beforehand.
    mInstance.reset();
}

bool DawnTestEnvironment::UsesWire() const {
    return mUseWire;
}

bool DawnTestEnvironment::RunSuppressedTests() const {
    return mRunSuppressedTests;
}

dawn::native::BackendValidationLevel DawnTestEnvironment::GetBackendValidationLevel() const {
    return mBackendValidationLevel;
}

dawn::native::Instance* DawnTestEnvironment::GetInstance() const {
    return mInstance.get();
}

bool DawnTestEnvironment::HasVendorIdFilter() const {
    return mHasVendorIdFilter;
}

uint32_t DawnTestEnvironment::GetVendorIdFilter() const {
    return mVendorIdFilter;
}

bool DawnTestEnvironment::HasBackendTypeFilter() const {
    return mHasBackendTypeFilter;
}

wgpu::BackendType DawnTestEnvironment::GetBackendTypeFilter() const {
    return mBackendTypeFilter;
}

const char* DawnTestEnvironment::GetWireTraceDir() const {
    if (mWireTraceDir.length() == 0) {
        return nullptr;
    }
    return mWireTraceDir.c_str();
}

const std::vector<std::string>& DawnTestEnvironment::GetEnabledToggles() const {
    return mToggleParser.GetEnabledToggles();
}

const std::vector<std::string>& DawnTestEnvironment::GetDisabledToggles() const {
    return mToggleParser.GetDisabledToggles();
}

// Implementation of DawnTest

DawnTestBase::DawnTestBase(const AdapterTestParam& param)
    : mParam(param),
      mWireHelper(utils::CreateWireHelper(gTestEnv->UsesWire(), gTestEnv->GetWireTraceDir())) {}

DawnTestBase::~DawnTestBase() {
    // We need to destroy child objects before the Device
    mReadbackSlots.clear();
    queue = wgpu::Queue();
    device = wgpu::Device();

    // D3D12's GPU-based validation will accumulate objects over time if the backend device is not
    // destroyed and recreated, so we reset it here.
    if (IsD3D12() && IsBackendValidationEnabled()) {
        mBackendAdapter.ResetInternalDeviceForTesting();
    }
    mWireHelper.reset();
}

bool DawnTestBase::IsD3D12() const {
    return mParam.adapterProperties.backendType == wgpu::BackendType::D3D12;
}

bool DawnTestBase::IsMetal() const {
    return mParam.adapterProperties.backendType == wgpu::BackendType::Metal;
}

bool DawnTestBase::IsNull() const {
    return mParam.adapterProperties.backendType == wgpu::BackendType::Null;
}

bool DawnTestBase::IsOpenGL() const {
    return mParam.adapterProperties.backendType == wgpu::BackendType::OpenGL;
}

bool DawnTestBase::IsOpenGLES() const {
    return mParam.adapterProperties.backendType == wgpu::BackendType::OpenGLES;
}

bool DawnTestBase::IsVulkan() const {
    return mParam.adapterProperties.backendType == wgpu::BackendType::Vulkan;
}

bool DawnTestBase::IsAMD() const {
    return gpu_info::IsAMD(mParam.adapterProperties.vendorID);
}

bool DawnTestBase::IsARM() const {
    return gpu_info::IsARM(mParam.adapterProperties.vendorID);
}

bool DawnTestBase::IsImgTec() const {
    return gpu_info::IsImgTec(mParam.adapterProperties.vendorID);
}

bool DawnTestBase::IsIntel() const {
    return gpu_info::IsIntel(mParam.adapterProperties.vendorID);
}

bool DawnTestBase::IsNvidia() const {
    return gpu_info::IsNvidia(mParam.adapterProperties.vendorID);
}

bool DawnTestBase::IsQualcomm() const {
    return gpu_info::IsQualcomm(mParam.adapterProperties.vendorID);
}

bool DawnTestBase::IsSwiftshader() const {
    return gpu_info::IsSwiftshader(mParam.adapterProperties.vendorID,
                                   mParam.adapterProperties.deviceID);
}

bool DawnTestBase::IsANGLE() const {
    return !mParam.adapterProperties.adapterName.find("ANGLE");
}

bool DawnTestBase::IsWARP() const {
    return gpu_info::IsWARP(mParam.adapterProperties.vendorID, mParam.adapterProperties.deviceID);
}

bool DawnTestBase::IsWindows() const {
#ifdef DAWN_PLATFORM_WINDOWS
    return true;
#else
    return false;
#endif
}

bool DawnTestBase::IsLinux() const {
#ifdef DAWN_PLATFORM_LINUX
    return true;
#else
    return false;
#endif
}

bool DawnTestBase::IsMacOS(int32_t majorVersion, int32_t minorVersion) const {
#ifdef DAWN_PLATFORM_MACOS
    if (majorVersion == -1 && minorVersion == -1) {
        return true;
    }
    int32_t majorVersionOut, minorVersionOut = 0;
    GetMacOSVersion(&majorVersionOut, &minorVersionOut);
    return (majorVersion != -1 && majorVersion == majorVersionOut) &&
           (minorVersion != -1 && minorVersion == minorVersionOut);
#else
    return false;
#endif
}

bool DawnTestBase::UsesWire() const {
    return gTestEnv->UsesWire();
}

bool DawnTestBase::IsBackendValidationEnabled() const {
    return gTestEnv->GetBackendValidationLevel() != dawn::native::BackendValidationLevel::Disabled;
}

bool DawnTestBase::RunSuppressedTests() const {
    return gTestEnv->RunSuppressedTests();
}

bool DawnTestBase::IsDXC() const {
    return HasToggleEnabled("use_dxc");
}

bool DawnTestBase::IsAsan() const {
#if defined(ADDRESS_SANITIZER)
    return true;
#else
    return false;
#endif
}

bool DawnTestBase::HasToggleEnabled(const char* toggle) const {
    auto toggles = dawn::native::GetTogglesUsed(backendDevice);
    return std::find_if(toggles.begin(), toggles.end(), [toggle](const char* name) {
               return strcmp(toggle, name) == 0;
           }) != toggles.end();
}

bool DawnTestBase::HasVendorIdFilter() const {
    return gTestEnv->HasVendorIdFilter();
}

uint32_t DawnTestBase::GetVendorIdFilter() const {
    return gTestEnv->GetVendorIdFilter();
}

bool DawnTestBase::HasBackendTypeFilter() const {
    return gTestEnv->HasBackendTypeFilter();
}

wgpu::BackendType DawnTestBase::GetBackendTypeFilter() const {
    return gTestEnv->GetBackendTypeFilter();
}

wgpu::Instance DawnTestBase::GetInstance() const {
    return gTestEnv->GetInstance()->Get();
}

dawn::native::Adapter DawnTestBase::GetAdapter() const {
    return mBackendAdapter;
}

std::vector<wgpu::FeatureName> DawnTestBase::GetRequiredFeatures() {
    return {};
}

wgpu::RequiredLimits DawnTestBase::GetRequiredLimits(const wgpu::SupportedLimits&) {
    return {};
}

const wgpu::AdapterProperties& DawnTestBase::GetAdapterProperties() const {
    return mParam.adapterProperties;
}

wgpu::SupportedLimits DawnTestBase::GetSupportedLimits() {
    WGPUSupportedLimits supportedLimits;
    supportedLimits.nextInChain = nullptr;
    dawn::native::GetProcs().deviceGetLimits(backendDevice, &supportedLimits);
    return *reinterpret_cast<wgpu::SupportedLimits*>(&supportedLimits);
}

bool DawnTestBase::SupportsFeatures(const std::vector<wgpu::FeatureName>& features) {
    ASSERT(mBackendAdapter);
    std::vector<wgpu::FeatureName> supportedFeatures;
    uint32_t count =
        dawn::native::GetProcs().adapterEnumerateFeatures(mBackendAdapter.Get(), nullptr);
    supportedFeatures.resize(count);
    dawn::native::GetProcs().adapterEnumerateFeatures(
        mBackendAdapter.Get(), reinterpret_cast<WGPUFeatureName*>(&supportedFeatures[0]));

    std::unordered_set<wgpu::FeatureName> supportedSet;
    for (wgpu::FeatureName f : supportedFeatures) {
        supportedSet.insert(f);
    }

    for (wgpu::FeatureName f : features) {
        if (supportedSet.count(f) == 0) {
            return false;
        }
    }
    return true;
}

std::pair<wgpu::Device, WGPUDevice> DawnTestBase::CreateDeviceImpl(std::string isolationKey) {
    // TODO(dawn:1399) Always flush wire before creating to avoid reuse of the same handle.
    FlushWire();

    // Create the device from the adapter
    for (const char* forceEnabledWorkaround : mParam.forceEnabledWorkarounds) {
        ASSERT(gTestEnv->GetInstance()->GetToggleInfo(forceEnabledWorkaround) != nullptr);
    }
    for (const char* forceDisabledWorkaround : mParam.forceDisabledWorkarounds) {
        ASSERT(gTestEnv->GetInstance()->GetToggleInfo(forceDisabledWorkaround) != nullptr);
    }

    std::vector<const char*> forceEnabledToggles = mParam.forceEnabledWorkarounds;
    std::vector<const char*> forceDisabledToggles = mParam.forceDisabledWorkarounds;

    std::vector<wgpu::FeatureName> requiredFeatures = GetRequiredFeatures();

    wgpu::SupportedLimits supportedLimits;
    mBackendAdapter.GetLimits(reinterpret_cast<WGPUSupportedLimits*>(&supportedLimits));
    wgpu::RequiredLimits requiredLimits = GetRequiredLimits(supportedLimits);

    // Disabled disallowing unsafe APIs so we can test them.
    forceDisabledToggles.push_back("disallow_unsafe_apis");

    for (const std::string& toggle : gTestEnv->GetEnabledToggles()) {
        const dawn::native::ToggleInfo* info =
            gTestEnv->GetInstance()->GetToggleInfo(toggle.c_str());
        ASSERT(info != nullptr);
        forceEnabledToggles.push_back(info->name);
    }

    for (const std::string& toggle : gTestEnv->GetDisabledToggles()) {
        const dawn::native::ToggleInfo* info =
            gTestEnv->GetInstance()->GetToggleInfo(toggle.c_str());
        ASSERT(info != nullptr);
        forceDisabledToggles.push_back(info->name);
    }

    wgpu::DeviceDescriptor deviceDescriptor = {};
    deviceDescriptor.requiredLimits = &requiredLimits;
    deviceDescriptor.requiredFeatures = requiredFeatures.data();
    deviceDescriptor.requiredFeaturesCount = requiredFeatures.size();

    wgpu::DawnTogglesDeviceDescriptor togglesDesc = {};
    deviceDescriptor.nextInChain = &togglesDesc;
    togglesDesc.forceEnabledToggles = forceEnabledToggles.data();
    togglesDesc.forceEnabledTogglesCount = forceEnabledToggles.size();
    togglesDesc.forceDisabledToggles = forceDisabledToggles.data();
    togglesDesc.forceDisabledTogglesCount = forceDisabledToggles.size();

    wgpu::DawnCacheDeviceDescriptor cacheDesc = {};
    togglesDesc.nextInChain = &cacheDesc;
    cacheDesc.isolationKey = isolationKey.c_str();

    auto devices = mWireHelper->RegisterDevice(mBackendAdapter.CreateDevice(&deviceDescriptor));
    wgpu::Device device = devices.first;
    device.SetUncapturedErrorCallback(mDeviceErrorCallback.Callback(),
                                      mDeviceErrorCallback.MakeUserdata(device.Get()));
    device.SetDeviceLostCallback(mDeviceLostCallback.Callback(),
                                 mDeviceLostCallback.MakeUserdata(device.Get()));
    device.SetLoggingCallback(
        [](WGPULoggingType type, char const* message, void*) {
            switch (type) {
                case WGPULoggingType_Verbose:
                    dawn::DebugLog() << message;
                    break;
                case WGPULoggingType_Warning:
                    dawn::WarningLog() << message;
                    break;
                case WGPULoggingType_Error:
                    dawn::ErrorLog() << message;
                    break;
                default:
                    dawn::InfoLog() << message;
                    break;
            }
        },
        nullptr);
    return devices;
}

wgpu::Device DawnTestBase::CreateDevice(std::string isolationKey) {
    return CreateDeviceImpl(isolationKey).first;
}

void DawnTestBase::SetUp() {
    {
        // Find the adapter that exactly matches our adapter properties.
        const auto& adapters = gTestEnv->GetInstance()->GetAdapters();
        const auto& it = std::find_if(
            adapters.begin(), adapters.end(), [&](const dawn::native::Adapter& adapter) {
                wgpu::AdapterProperties properties;
                adapter.GetProperties(&properties);

                return (mParam.adapterProperties.selected &&
                        properties.deviceID == mParam.adapterProperties.deviceID &&
                        properties.vendorID == mParam.adapterProperties.vendorID &&
                        properties.adapterType == mParam.adapterProperties.adapterType &&
                        properties.backendType == mParam.adapterProperties.backendType &&
                        strcmp(properties.name, mParam.adapterProperties.adapterName.c_str()) == 0);
            });
        ASSERT(it != adapters.end());
        mBackendAdapter = *it;
    }

    // Setup the per-test platform. Tests can provide one by overloading CreateTestPlatform. This is
    // NOT a thread-safe operation and is allowed here for testing only.
    mTestPlatform = CreateTestPlatform();
    dawn::native::FromAPI(gTestEnv->GetInstance()->Get())
        ->SetPlatformForTesting(mTestPlatform.get());

    std::string traceName =
        std::string(::testing::UnitTest::GetInstance()->current_test_info()->test_suite_name()) +
        "_" + ::testing::UnitTest::GetInstance()->current_test_info()->name();
    mWireHelper->BeginWireTrace(traceName.c_str());

    // Create the device from the adapter
    std::tie(device, backendDevice) = CreateDeviceImpl();
    ASSERT_NE(nullptr, backendDevice);

    queue = device.GetQueue();

#if defined(DAWN_ENABLE_BACKEND_DESKTOP_GL)
    if (IsOpenGL()) {
        glfwMakeContextCurrent(gTestEnv->GetOpenGLWindow());
    }
#endif  // defined(DAWN_ENABLE_BACKEND_DESKTOP_GL)
#if defined(DAWN_ENABLE_BACKEND_OPENGLES)
    if (IsOpenGLES()) {
        glfwMakeContextCurrent(gTestEnv->GetOpenGLESWindow());
    }
#endif  // defined(DAWN_ENABLE_BACKEND_OPENGLES)
}

void DawnTestBase::TearDown() {
    FlushWire();

    MapSlotsSynchronously();
    ResolveExpectations();

    for (size_t i = 0; i < mReadbackSlots.size(); ++i) {
        mReadbackSlots[i].buffer.Unmap();
    }

    if (!UsesWire()) {
        EXPECT_EQ(mLastWarningCount,
                  dawn::native::GetDeprecationWarningCountForTesting(device.Get()));
    }
}

void DawnTestBase::DestroyDevice(wgpu::Device device) {
    wgpu::Device resolvedDevice;
    if (device != nullptr) {
        resolvedDevice = device;
    } else {
        resolvedDevice = this->device;
    }
    EXPECT_CALL(mDeviceLostCallback,
                Call(WGPUDeviceLostReason_Destroyed, testing::_, resolvedDevice.Get()))
        .Times(1);
    resolvedDevice.Destroy();
    FlushWire();
    testing::Mock::VerifyAndClearExpectations(&mDeviceLostCallback);
}

void DawnTestBase::LoseDeviceForTesting(wgpu::Device device) {
    wgpu::Device resolvedDevice;
    if (device != nullptr) {
        resolvedDevice = device;
    } else {
        resolvedDevice = this->device;
    }
    EXPECT_CALL(mDeviceLostCallback,
                Call(WGPUDeviceLostReason_Undefined, testing::_, resolvedDevice.Get()))
        .Times(1);
    resolvedDevice.LoseForTesting();
    FlushWire();
    testing::Mock::VerifyAndClearExpectations(&mDeviceLostCallback);
}

std::ostringstream& DawnTestBase::AddBufferExpectation(const char* file,
                                                       int line,
                                                       const wgpu::Buffer& buffer,
                                                       uint64_t offset,
                                                       uint64_t size,
                                                       detail::Expectation* expectation) {
    uint64_t alignedSize = Align(size, uint64_t(4));
    auto readback = ReserveReadback(alignedSize);

    // We need to enqueue the copy immediately because by the time we resolve the expectation,
    // the buffer might have been modified.
    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    encoder.CopyBufferToBuffer(buffer, offset, readback.buffer, readback.offset, alignedSize);

    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    DeferredExpectation deferred;
    deferred.file = file;
    deferred.line = line;
    deferred.readbackSlot = readback.slot;
    deferred.readbackOffset = readback.offset;
    deferred.size = size;
    deferred.expectation.reset(expectation);

    mDeferredExpectations.push_back(std::move(deferred));
    mDeferredExpectations.back().message = std::make_unique<std::ostringstream>();
    return *(mDeferredExpectations.back().message.get());
}

std::ostringstream& DawnTestBase::AddTextureExpectationImpl(const char* file,
                                                            int line,
                                                            detail::Expectation* expectation,
                                                            const wgpu::Texture& texture,
                                                            wgpu::Origin3D origin,
                                                            wgpu::Extent3D extent,
                                                            uint32_t level,
                                                            wgpu::TextureAspect aspect,
                                                            uint32_t dataSize,
                                                            uint32_t bytesPerRow) {
    if (bytesPerRow == 0) {
        bytesPerRow = Align(extent.width * dataSize, kTextureBytesPerRowAlignment);
    } else {
        ASSERT(bytesPerRow >= extent.width * dataSize);
        ASSERT(bytesPerRow == Align(bytesPerRow, kTextureBytesPerRowAlignment));
    }

    uint32_t rowsPerImage = extent.height;
    uint32_t size = utils::RequiredBytesInCopy(bytesPerRow, rowsPerImage, extent.width,
                                               extent.height, extent.depthOrArrayLayers, dataSize);

    auto readback = ReserveReadback(Align(size, 4));

    // We need to enqueue the copy immediately because by the time we resolve the expectation,
    // the texture might have been modified.
    wgpu::ImageCopyTexture imageCopyTexture =
        utils::CreateImageCopyTexture(texture, level, origin, aspect);
    wgpu::ImageCopyBuffer imageCopyBuffer =
        utils::CreateImageCopyBuffer(readback.buffer, readback.offset, bytesPerRow, rowsPerImage);

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    encoder.CopyTextureToBuffer(&imageCopyTexture, &imageCopyBuffer, &extent);

    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    DeferredExpectation deferred;
    deferred.file = file;
    deferred.line = line;
    deferred.readbackSlot = readback.slot;
    deferred.readbackOffset = readback.offset;
    deferred.size = size;
    deferred.rowBytes = extent.width * dataSize;
    deferred.bytesPerRow = bytesPerRow;
    deferred.expectation.reset(expectation);

    mDeferredExpectations.push_back(std::move(deferred));
    mDeferredExpectations.back().message = std::make_unique<std::ostringstream>();
    return *(mDeferredExpectations.back().message.get());
}

std::ostringstream& DawnTestBase::ExpectSampledFloatDataImpl(wgpu::TextureView textureView,
                                                             const char* wgslTextureType,
                                                             uint32_t width,
                                                             uint32_t height,
                                                             uint32_t componentCount,
                                                             uint32_t sampleCount,
                                                             detail::Expectation* expectation) {
    std::ostringstream shaderSource;
    shaderSource << "let width : u32 = " << width << "u;\n";
    shaderSource << "@group(0) @binding(0) var tex : " << wgslTextureType << ";\n";
    shaderSource << R"(
        struct Result {
            values : array<f32>
        }
        @group(0) @binding(1) var<storage, read_write> result : Result;
    )";
    shaderSource << "let componentCount : u32 = " << componentCount << "u;\n";
    shaderSource << "let sampleCount : u32 = " << sampleCount << "u;\n";

    shaderSource << "fn doTextureLoad(t: " << wgslTextureType
                 << ", coord: vec2<i32>, sample: u32, component: u32) -> f32";
    if (sampleCount > 1) {
        shaderSource << R"({
            return textureLoad(tex, coord, i32(sample))[component];
        })";
    } else {
        if (strcmp(wgslTextureType, "texture_depth_2d") == 0) {
            ASSERT(componentCount == 1);
            shaderSource << R"({
                return textureLoad(tex, coord, 0);
            })";
        } else {
            shaderSource << R"({
                return textureLoad(tex, coord, 0)[component];
            })";
        }
    }
    shaderSource << R"(
        @stage(compute) @workgroup_size(1) fn main(
            @builtin(global_invocation_id) GlobalInvocationId : vec3<u32>
        ) {
            let baseOutIndex = GlobalInvocationId.y * width + GlobalInvocationId.x;
            for (var s = 0u; s < sampleCount; s = s + 1u) {
                for (var c = 0u; c < componentCount; c = c + 1u) {
                    result.values[
                        baseOutIndex * sampleCount * componentCount +
                        s * componentCount +
                        c
                    ] = doTextureLoad(tex, vec2<i32>(GlobalInvocationId.xy), s, c);
                }
            }
        }
    )";

    wgpu::ShaderModule csModule = utils::CreateShaderModule(device, shaderSource.str().c_str());

    wgpu::ComputePipelineDescriptor pipelineDescriptor;
    pipelineDescriptor.compute.module = csModule;
    pipelineDescriptor.compute.entryPoint = "main";

    wgpu::ComputePipeline pipeline = device.CreateComputePipeline(&pipelineDescriptor);

    // Create and initialize the slot buffer so that it won't unexpectedly affect the count of
    // resources lazily cleared.
    const std::vector<float> initialBufferData(width * height * componentCount * sampleCount, 0.f);
    wgpu::Buffer readbackBuffer = utils::CreateBufferFromData(
        device, initialBufferData.data(), sizeof(float) * initialBufferData.size(),
        wgpu::BufferUsage::CopySrc | wgpu::BufferUsage::Storage);

    wgpu::BindGroup bindGroup = utils::MakeBindGroup(device, pipeline.GetBindGroupLayout(0),
                                                     {{0, textureView}, {1, readbackBuffer}});

    wgpu::CommandEncoder commandEncoder = device.CreateCommandEncoder();
    wgpu::ComputePassEncoder pass = commandEncoder.BeginComputePass();
    pass.SetPipeline(pipeline);
    pass.SetBindGroup(0, bindGroup);
    pass.DispatchWorkgroups(width, height);
    pass.End();
    wgpu::CommandBuffer commands = commandEncoder.Finish();
    queue.Submit(1, &commands);

    return EXPECT_BUFFER(readbackBuffer, 0, initialBufferData.size() * sizeof(float), expectation);
}

std::ostringstream& DawnTestBase::ExpectSampledFloatData(wgpu::Texture texture,
                                                         uint32_t width,
                                                         uint32_t height,
                                                         uint32_t componentCount,
                                                         uint32_t arrayLayer,
                                                         uint32_t mipLevel,
                                                         detail::Expectation* expectation) {
    wgpu::TextureViewDescriptor viewDesc = {};
    viewDesc.dimension = wgpu::TextureViewDimension::e2D;
    viewDesc.baseMipLevel = mipLevel;
    viewDesc.mipLevelCount = 1;
    viewDesc.baseArrayLayer = arrayLayer;
    viewDesc.arrayLayerCount = 1;

    return ExpectSampledFloatDataImpl(texture.CreateView(&viewDesc), "texture_2d<f32>", width,
                                      height, componentCount, 1, expectation);
}

std::ostringstream& DawnTestBase::ExpectMultisampledFloatData(wgpu::Texture texture,
                                                              uint32_t width,
                                                              uint32_t height,
                                                              uint32_t componentCount,
                                                              uint32_t sampleCount,
                                                              uint32_t arrayLayer,
                                                              uint32_t mipLevel,
                                                              detail::Expectation* expectation) {
    wgpu::TextureViewDescriptor viewDesc = {};
    viewDesc.dimension = wgpu::TextureViewDimension::e2D;
    viewDesc.baseMipLevel = mipLevel;
    viewDesc.mipLevelCount = 1;
    viewDesc.baseArrayLayer = arrayLayer;
    viewDesc.arrayLayerCount = 1;

    return ExpectSampledFloatDataImpl(texture.CreateView(&viewDesc), "texture_multisampled_2d<f32>",
                                      width, height, componentCount, sampleCount, expectation);
}

std::ostringstream& DawnTestBase::ExpectSampledDepthData(wgpu::Texture texture,
                                                         uint32_t width,
                                                         uint32_t height,
                                                         uint32_t arrayLayer,
                                                         uint32_t mipLevel,
                                                         detail::Expectation* expectation) {
    wgpu::TextureViewDescriptor viewDesc = {};
    viewDesc.aspect = wgpu::TextureAspect::DepthOnly;
    viewDesc.dimension = wgpu::TextureViewDimension::e2D;
    viewDesc.baseMipLevel = mipLevel;
    viewDesc.mipLevelCount = 1;
    viewDesc.baseArrayLayer = arrayLayer;
    viewDesc.arrayLayerCount = 1;

    return ExpectSampledFloatDataImpl(texture.CreateView(&viewDesc), "texture_depth_2d", width,
                                      height, 1, 1, expectation);
}

std::ostringstream& DawnTestBase::ExpectAttachmentDepthStencilTestData(
    wgpu::Texture texture,
    wgpu::TextureFormat format,
    uint32_t width,
    uint32_t height,
    uint32_t arrayLayer,
    uint32_t mipLevel,
    std::vector<float> expectedDepth,
    uint8_t* expectedStencil) {
    wgpu::CommandEncoder commandEncoder = device.CreateCommandEncoder();

    // Make the color attachment that we'll use to read back.
    wgpu::TextureDescriptor colorTexDesc = {};
    colorTexDesc.size = {width, height, 1};
    colorTexDesc.format = wgpu::TextureFormat::R32Uint;
    colorTexDesc.usage = wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::CopySrc;
    wgpu::Texture colorTexture = device.CreateTexture(&colorTexDesc);

    wgpu::Texture depthDataTexture = nullptr;
    if (expectedDepth.size() > 0) {
        // Make a sampleable texture to store the depth data. We'll sample this in the
        // shader to output depth.
        wgpu::TextureDescriptor depthDataDesc = {};
        depthDataDesc.size = {width, height, 1};
        depthDataDesc.format = wgpu::TextureFormat::R32Float;
        depthDataDesc.usage = wgpu::TextureUsage::TextureBinding | wgpu::TextureUsage::CopyDst;
        depthDataTexture = device.CreateTexture(&depthDataDesc);

        // Upload the depth data.
        wgpu::ImageCopyTexture imageCopyTexture =
            utils::CreateImageCopyTexture(depthDataTexture, 0, {0, 0, 0});
        wgpu::TextureDataLayout textureDataLayout =
            utils::CreateTextureDataLayout(0, sizeof(float) * width);
        wgpu::Extent3D copyExtent = {width, height, 1};

        queue.WriteTexture(&imageCopyTexture, expectedDepth.data(),
                           sizeof(float) * expectedDepth.size(), &textureDataLayout, &copyExtent);
    }

    // Pipeline for a full screen quad.
    utils::ComboRenderPipelineDescriptor pipelineDescriptor;

    pipelineDescriptor.vertex.module = utils::CreateShaderModule(device, R"(
        @stage(vertex)
        fn main(@builtin(vertex_index) VertexIndex : u32) -> @builtin(position) vec4<f32> {
            var pos = array<vec2<f32>, 3>(
                vec2<f32>(-1.0, -1.0),
                vec2<f32>( 3.0, -1.0),
                vec2<f32>(-1.0,  3.0));
            return vec4<f32>(pos[VertexIndex], 0.0, 1.0);
        })");

    if (depthDataTexture) {
        // Sample the input texture and write out depth. |result| will only be set to 1 if we
        // pass the depth test.
        pipelineDescriptor.cFragment.module = utils::CreateShaderModule(device, R"(
            @group(0) @binding(0) var texture0 : texture_2d<f32>;

            struct FragmentOut {
                @location(0) result : u32,
                @builtin(frag_depth) fragDepth : f32,
            }

            @stage(fragment)
            fn main(@builtin(position) FragCoord : vec4<f32>) -> FragmentOut {
                var output : FragmentOut;
                output.result = 1u;
                output.fragDepth = textureLoad(texture0, vec2<i32>(FragCoord.xy), 0)[0];
                return output;
            })");
    } else {
        pipelineDescriptor.cFragment.module = utils::CreateShaderModule(device, R"(
            @stage(fragment)
            fn main() -> @location(0) u32 {
                return 1u;
            })");
    }

    wgpu::DepthStencilState* depthStencil = pipelineDescriptor.EnableDepthStencil(format);
    if (depthDataTexture) {
        // Pass the depth test only if the depth is equal.
        depthStencil->depthCompare = wgpu::CompareFunction::Equal;
    }

    if (expectedStencil != nullptr) {
        // Pass the stencil test only if the stencil is equal.
        depthStencil->stencilFront.compare = wgpu::CompareFunction::Equal;
    }

    pipelineDescriptor.cTargets[0].format = colorTexDesc.format;

    wgpu::TextureViewDescriptor viewDesc = {};
    viewDesc.baseMipLevel = mipLevel;
    viewDesc.mipLevelCount = 1;
    viewDesc.baseArrayLayer = arrayLayer;
    viewDesc.arrayLayerCount = 1;

    utils::ComboRenderPassDescriptor passDescriptor({colorTexture.CreateView()},
                                                    texture.CreateView(&viewDesc));
    passDescriptor.cDepthStencilAttachmentInfo.depthLoadOp = wgpu::LoadOp::Load;
    passDescriptor.cDepthStencilAttachmentInfo.stencilLoadOp = wgpu::LoadOp::Load;
    switch (format) {
        case wgpu::TextureFormat::Depth24Plus:
        case wgpu::TextureFormat::Depth32Float:
        case wgpu::TextureFormat::Depth16Unorm:
            passDescriptor.cDepthStencilAttachmentInfo.stencilLoadOp = wgpu::LoadOp::Undefined;
            passDescriptor.cDepthStencilAttachmentInfo.stencilStoreOp = wgpu::StoreOp::Undefined;
            break;
        case wgpu::TextureFormat::Stencil8:
            passDescriptor.cDepthStencilAttachmentInfo.depthLoadOp = wgpu::LoadOp::Undefined;
            passDescriptor.cDepthStencilAttachmentInfo.depthStoreOp = wgpu::StoreOp::Undefined;
            break;
        default:
            break;
    }

    wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&pipelineDescriptor);

    wgpu::RenderPassEncoder pass = commandEncoder.BeginRenderPass(&passDescriptor);
    if (expectedStencil != nullptr) {
        pass.SetStencilReference(*expectedStencil);
    }
    pass.SetPipeline(pipeline);
    if (depthDataTexture) {
        // Bind the depth data texture.
        pass.SetBindGroup(0, utils::MakeBindGroup(device, pipeline.GetBindGroupLayout(0),
                                                  {{0, depthDataTexture.CreateView()}}));
    }
    pass.Draw(3);
    pass.End();

    wgpu::CommandBuffer commands = commandEncoder.Finish();
    queue.Submit(1, &commands);

    std::vector<uint32_t> colorData(width * height, 1u);
    return EXPECT_TEXTURE_EQ(colorData.data(), colorTexture, {0, 0}, {width, height});
}

void DawnTestBase::WaitABit() {
    device.Tick();
    FlushWire();

    utils::USleep(100);
}

void DawnTestBase::FlushWire() {
    if (gTestEnv->UsesWire()) {
        bool C2SFlushed = mWireHelper->FlushClient();
        bool S2CFlushed = mWireHelper->FlushServer();
        ASSERT(C2SFlushed);
        ASSERT(S2CFlushed);
    }
}

void DawnTestBase::WaitForAllOperations() {
    bool done = false;
    device.GetQueue().OnSubmittedWorkDone(
        0u, [](WGPUQueueWorkDoneStatus, void* userdata) { *static_cast<bool*>(userdata) = true; },
        &done);
    while (!done) {
        WaitABit();
    }
}

DawnTestBase::ReadbackReservation DawnTestBase::ReserveReadback(uint64_t readbackSize) {
    ReadbackSlot slot;
    slot.bufferSize = readbackSize;

    // Create and initialize the slot buffer so that it won't unexpectedly affect the count of
    // resource lazy clear in the tests.
    const std::vector<uint8_t> initialBufferData(readbackSize, 0u);
    slot.buffer =
        utils::CreateBufferFromData(device, initialBufferData.data(), readbackSize,
                                    wgpu::BufferUsage::MapRead | wgpu::BufferUsage::CopyDst);

    ReadbackReservation reservation;
    reservation.buffer = slot.buffer;
    reservation.slot = mReadbackSlots.size();
    reservation.offset = 0;

    mReadbackSlots.push_back(std::move(slot));
    return reservation;
}

void DawnTestBase::MapSlotsSynchronously() {
    // Initialize numPendingMapOperations before mapping, just in case the callback is called
    // immediately.
    mNumPendingMapOperations = mReadbackSlots.size();

    // Map all readback slots
    for (size_t i = 0; i < mReadbackSlots.size(); ++i) {
        MapReadUserdata* userdata = new MapReadUserdata{this, i};

        const ReadbackSlot& slot = mReadbackSlots[i];
        slot.buffer.MapAsync(wgpu::MapMode::Read, 0, wgpu::kWholeMapSize, SlotMapCallback,
                             userdata);
    }

    // Busy wait until all map operations are done.
    while (mNumPendingMapOperations != 0) {
        WaitABit();
    }
}

// static
void DawnTestBase::SlotMapCallback(WGPUBufferMapAsyncStatus status, void* userdata_) {
    DAWN_ASSERT(status == WGPUBufferMapAsyncStatus_Success);

    std::unique_ptr<MapReadUserdata> userdata(static_cast<MapReadUserdata*>(userdata_));
    DawnTestBase* test = userdata->test;
    ReadbackSlot* slot = &test->mReadbackSlots[userdata->slot];

    slot->mappedData = slot->buffer.GetConstMappedRange();
    test->mNumPendingMapOperations--;
}

void DawnTestBase::ResolveExpectations() {
    for (const auto& expectation : mDeferredExpectations) {
        DAWN_ASSERT(mReadbackSlots[expectation.readbackSlot].mappedData != nullptr);

        // Get a pointer to the mapped copy of the data for the expectation.
        const char* data =
            static_cast<const char*>(mReadbackSlots[expectation.readbackSlot].mappedData);
        data += expectation.readbackOffset;

        uint32_t size;
        std::vector<char> packedData;
        if (expectation.rowBytes != expectation.bytesPerRow) {
            DAWN_ASSERT(expectation.bytesPerRow > expectation.rowBytes);
            uint32_t rowCount =
                (expectation.size + expectation.bytesPerRow - 1) / expectation.bytesPerRow;
            uint32_t packedSize = rowCount * expectation.rowBytes;
            packedData.resize(packedSize);
            for (uint32_t r = 0; r < rowCount; ++r) {
                for (uint32_t i = 0; i < expectation.rowBytes; ++i) {
                    packedData[i + r * expectation.rowBytes] =
                        data[i + r * expectation.bytesPerRow];
                }
            }
            data = packedData.data();
            size = packedSize;
        } else {
            size = expectation.size;
        }

        // Get the result for the expectation and add context to failures
        testing::AssertionResult result = expectation.expectation->Check(data, size);
        if (!result) {
            result << " Expectation created at " << expectation.file << ":" << expectation.line
                   << std::endl;
            result << expectation.message->str();
        }

        EXPECT_TRUE(result);
    }
}

std::unique_ptr<dawn::platform::Platform> DawnTestBase::CreateTestPlatform() {
    return nullptr;
}

bool RGBA8::operator==(const RGBA8& other) const {
    return r == other.r && g == other.g && b == other.b && a == other.a;
}

bool RGBA8::operator!=(const RGBA8& other) const {
    return !(*this == other);
}

bool RGBA8::operator<=(const RGBA8& other) const {
    return (r <= other.r && g <= other.g && b <= other.b && a <= other.a);
}

bool RGBA8::operator>=(const RGBA8& other) const {
    return (r >= other.r && g >= other.g && b >= other.b && a >= other.a);
}

std::ostream& operator<<(std::ostream& stream, const RGBA8& color) {
    return stream << "RGBA8(" << static_cast<int>(color.r) << ", " << static_cast<int>(color.g)
                  << ", " << static_cast<int>(color.b) << ", " << static_cast<int>(color.a) << ")";
}

namespace detail {
std::vector<AdapterTestParam> GetAvailableAdapterTestParamsForBackends(
    const BackendTestConfig* params,
    size_t numParams) {
    ASSERT(gTestEnv != nullptr);
    return gTestEnv->GetAvailableAdapterTestParamsForBackends(params, numParams);
}

// Helper classes to set expectations

template <typename T, typename U>
ExpectEq<T, U>::ExpectEq(T singleValue, T tolerance) : mTolerance(tolerance) {
    mExpected.push_back(singleValue);
}

template <typename T, typename U>
ExpectEq<T, U>::ExpectEq(const T* values, const unsigned int count, T tolerance)
    : mTolerance(tolerance) {
    mExpected.assign(values, values + count);
}

namespace {

template <typename T, typename U = T>
testing::AssertionResult CheckImpl(const T& expected, const U& actual, const T& tolerance) {
    ASSERT(tolerance == T{});
    if (expected != actual) {
        return testing::AssertionFailure() << expected << ", actual " << actual;
    }
    return testing::AssertionSuccess();
}

template <>
testing::AssertionResult CheckImpl<float>(const float& expected,
                                          const float& actual,
                                          const float& tolerance) {
    if (abs(expected - actual) > tolerance) {
        return tolerance == 0.0 ? testing::AssertionFailure() << expected << ", actual " << actual
                                : testing::AssertionFailure() << "within " << tolerance << " of "
                                                              << expected << ", actual " << actual;
    }
    return testing::AssertionSuccess();
}

// Interpret uint16_t as float16
// This is mostly for reading float16 output from textures
template <>
testing::AssertionResult CheckImpl<float, uint16_t>(const float& expected,
                                                    const uint16_t& actual,
                                                    const float& tolerance) {
    float actualF32 = Float16ToFloat32(actual);
    if (abs(expected - actualF32) > tolerance) {
        return tolerance == 0.0
                   ? testing::AssertionFailure() << expected << ", actual " << actualF32
                   : testing::AssertionFailure() << "within " << tolerance << " of " << expected
                                                 << ", actual " << actualF32;
    }
    return testing::AssertionSuccess();
}

}  // namespace

template <typename T, typename U>
testing::AssertionResult ExpectEq<T, U>::Check(const void* data, size_t size) {
    DAWN_ASSERT(size == sizeof(U) * mExpected.size());
    const U* actual = static_cast<const U*>(data);

    for (size_t i = 0; i < mExpected.size(); ++i) {
        testing::AssertionResult check = CheckImpl(mExpected[i], actual[i], mTolerance);
        if (!check) {
            testing::AssertionResult result = testing::AssertionFailure()
                                              << "Expected data[" << i << "] to be "
                                              << check.message() << std::endl;

            if (mExpected.size() <= 1024) {
                result << "Expected:" << std::endl;
                printBuffer(result, mExpected.data(), mExpected.size());

                result << "Actual:" << std::endl;
                printBuffer(result, actual, mExpected.size());
            }

            return result;
        }
    }
    return testing::AssertionSuccess();
}

template class ExpectEq<uint8_t>;
template class ExpectEq<uint16_t>;
template class ExpectEq<uint32_t>;
template class ExpectEq<uint64_t>;
template class ExpectEq<RGBA8>;
template class ExpectEq<float>;
template class ExpectEq<float, uint16_t>;

template <typename T>
ExpectBetweenColors<T>::ExpectBetweenColors(T value0, T value1) {
    T l, h;
    l.r = std::min(value0.r, value1.r);
    l.g = std::min(value0.g, value1.g);
    l.b = std::min(value0.b, value1.b);
    l.a = std::min(value0.a, value1.a);

    h.r = std::max(value0.r, value1.r);
    h.g = std::max(value0.g, value1.g);
    h.b = std::max(value0.b, value1.b);
    h.a = std::max(value0.a, value1.a);

    mLowerColorChannels.push_back(l);
    mHigherColorChannels.push_back(h);

    mValues0.push_back(value0);
    mValues1.push_back(value1);
}

template <typename T>
testing::AssertionResult ExpectBetweenColors<T>::Check(const void* data, size_t size) {
    DAWN_ASSERT(size == sizeof(T) * mLowerColorChannels.size());
    DAWN_ASSERT(mHigherColorChannels.size() == mLowerColorChannels.size());
    DAWN_ASSERT(mValues0.size() == mValues1.size());
    DAWN_ASSERT(mValues0.size() == mLowerColorChannels.size());

    const T* actual = static_cast<const T*>(data);

    for (size_t i = 0; i < mLowerColorChannels.size(); ++i) {
        if (!(actual[i] >= mLowerColorChannels[i] && actual[i] <= mHigherColorChannels[i])) {
            testing::AssertionResult result = testing::AssertionFailure()
                                              << "Expected data[" << i << "] to be between "
                                              << mValues0[i] << " and " << mValues1[i]
                                              << ", actual " << actual[i] << std::endl;

            if (mLowerColorChannels.size() <= 1024) {
                result << "Expected between:" << std::endl;
                printBuffer(result, mValues0.data(), mLowerColorChannels.size());
                result << "and" << std::endl;
                printBuffer(result, mValues1.data(), mLowerColorChannels.size());

                result << "Actual:" << std::endl;
                printBuffer(result, actual, mLowerColorChannels.size());
            }

            return result;
        }
    }

    return testing::AssertionSuccess();
}

template class ExpectBetweenColors<RGBA8>;
}  // namespace detail

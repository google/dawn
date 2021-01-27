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

#include "tests/DawnTest.h"

#include "common/Assert.h"
#include "common/GPUInfo.h"
#include "common/Log.h"
#include "common/Math.h"
#include "common/Platform.h"
#include "common/SystemUtils.h"
#include "dawn/dawn_proc.h"
#include "dawn_native/DawnNative.h"
#include "dawn_wire/WireClient.h"
#include "dawn_wire/WireServer.h"
#include "utils/PlatformDebugLogger.h"
#include "utils/SystemUtils.h"
#include "utils/TerribleCommandBuffer.h"
#include "utils/TestUtils.h"
#include "utils/WGPUHelpers.h"

#include <algorithm>
#include <fstream>
#include <iomanip>
#include <regex>
#include <sstream>
#include <unordered_map>

#if defined(DAWN_ENABLE_BACKEND_OPENGL)
#    include "GLFW/glfw3.h"
#    include "dawn_native/OpenGLBackend.h"
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
                sprintf(buf, "%02X ", byteView[b]);
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
      forceDisabledWorkarounds(forceDisabledWorkarounds) {
}

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
    : wgpu::AdapterProperties(properties), adapterName(properties.name), selected(selected) {
}

AdapterTestParam::AdapterTestParam(const BackendTestConfig& config,
                                   const TestAdapterProperties& adapterProperties)
    : adapterProperties(adapterProperties),
      forceEnabledWorkarounds(config.forceEnabledWorkarounds),
      forceDisabledWorkarounds(config.forceDisabledWorkarounds) {
}

std::ostream& operator<<(std::ostream& os, const AdapterTestParam& param) {
    // Sanitize the adapter name for GoogleTest
    std::string sanitizedName =
        std::regex_replace(param.adapterProperties.adapterName, std::regex("[^a-zA-Z0-9]+"), "_");

    // Strip trailing underscores, if any.
    if (sanitizedName.back() == '_') {
        sanitizedName.back() = '\0';
    }

    os << ParamName(param.adapterProperties.backendType) << "_" << sanitizedName.c_str();

    // In a Windows Remote Desktop session there are two adapters named "Microsoft Basic Render
    // Driver" with different adapter types. We must differentiate them to avoid any tests using the
    // same name.
    if (param.adapterProperties.deviceID == 0x008C) {
        std::string adapterType = AdapterTypeName(param.adapterProperties.adapterType);
        std::replace(adapterType.begin(), adapterType.end(), ' ', '_');
        os << "_" << adapterType;
    }

    for (const char* forceEnabledWorkaround : param.forceEnabledWorkarounds) {
        os << "__e_" << forceEnabledWorkaround;
    }
    for (const char* forceDisabledWorkaround : param.forceDisabledWorkarounds) {
        os << "__d_" << forceDisabledWorkaround;
    }
    return os;
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

    if (mEnableBackendValidation) {
        mPlatformDebugLogger =
            std::unique_ptr<utils::PlatformDebugLogger>(utils::CreatePlatformDebugLogger());
    }

    // Create a temporary instance to select available and preferred adapters. This is done before
    // test instantiation so GetAvailableAdapterTestParamsForBackends can generate test
    // parameterizations all selected adapters. We drop the instance at the end of this function
    // because the Vulkan validation layers use static global mutexes which behave badly when
    // Chromium's test launcher forks the test process. The instance will be recreated on test
    // environment setup.
    std::unique_ptr<dawn_native::Instance> instance = CreateInstanceAndDiscoverAdapters();
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

        if (strcmp("-d", argv[i]) == 0 || strcmp("--enable-backend-validation", argv[i]) == 0) {
            mEnableBackendValidation = true;
            continue;
        }

        if (strcmp("-c", argv[i]) == 0 || strcmp("--begin-capture-on-startup", argv[i]) == 0) {
            mBeginCaptureOnStartup = true;
            continue;
        }

        constexpr const char kEnableTogglesSwitch[] = "--enable-toggles=";
        argLen = sizeof(kEnableTogglesSwitch) - 1;
        if (strncmp(argv[i], kEnableTogglesSwitch, argLen) == 0) {
            std::string toggle;
            std::stringstream toggles(argv[i] + argLen);
            while (getline(toggles, toggle, ',')) {
                mEnabledToggles.push_back(toggle);
            }
            continue;
        }

        constexpr const char kDisableTogglesSwitch[] = "--disable-toggles=";
        argLen = sizeof(kDisableTogglesSwitch) - 1;
        if (strncmp(argv[i], kDisableTogglesSwitch, argLen) == 0) {
            std::string toggle;
            std::stringstream toggles(argv[i] + argLen);
            while (getline(toggles, toggle, ',')) {
                mDisabledToggles.push_back(toggle);
            }
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
                        mDevicePreferences.push_back(dawn_native::DeviceType::DiscreteGPU);
                    } else if (strcmp(type.c_str(), "integrated") == 0) {
                        mDevicePreferences.push_back(dawn_native::DeviceType::IntegratedGPU);
                    } else if (strcmp(type.c_str(), "cpu") == 0) {
                        mDevicePreferences.push_back(dawn_native::DeviceType::CPU);
                    } else {
                        dawn::ErrorLog() << "Invalid device type preference: " << type;
                        UNREACHABLE();
                    }
                }
            }
        }

        constexpr const char kWireTraceDirArg[] = "--wire-trace-dir=";
        argLen = sizeof(kWireTraceDirArg) - 1;
        if (strncmp(argv[i], kWireTraceDirArg, argLen) == 0) {
            const char* wireTraceDir = argv[i] + argLen;
            if (wireTraceDir[0] != '\0') {
                const char* sep = GetPathSeparator();
                mWireTraceDir = wireTraceDir;
                if (mWireTraceDir.back() != *sep) {
                    mWireTraceDir += sep;
                }
            }
            continue;
        }

        if (strcmp("-h", argv[i]) == 0 || strcmp("--help", argv[i]) == 0) {
            dawn::InfoLog()
                << "\n\nUsage: " << argv[0]
                << " [GTEST_FLAGS...] [-w] [-d] [-c]\n"
                   "    [--enable-toggles=toggles] [--disable-toggles=toggles]\n"
                   "    [--adapter-vendor-id=x]"
                   " [--exclusive-device-type-preference=integrated,cpu,discrete]\n\n"
                   "  -w, --use-wire: Run the tests through the wire (defaults to no wire)\n"
                   "  -d, --enable-backend-validation: Enable backend validation (defaults"
                   " to disabled)\n"
                   "  -c, --begin-capture-on-startup: Begin debug capture on startup "
                   "(defaults to no capture)\n"
                   "  --enable-toggles: Comma-delimited list of Dawn toggles to enable.\n"
                   "    ex.) skip_validation,use_tint_generator,disable_robustness,turn_off_vsync\n"
                   "  --disable-toggles: Comma-delimited list of Dawn toggles to disable\n"
                   "  --adapter-vendor-id: Select adapter by vendor id to run end2end tests"
                   "on multi-GPU systems \n"
                   "  --exclusive-device-type-preference: Comma-delimited list of preferred device "
                   "types. For each backend, tests will run only on adapters that match the first "
                   "available device type\n";
            continue;
        }
    }
}

std::unique_ptr<dawn_native::Instance> DawnTestEnvironment::CreateInstanceAndDiscoverAdapters() {
    auto instance = std::make_unique<dawn_native::Instance>();
    instance->EnableBackendValidation(mEnableBackendValidation);
    instance->EnableGPUBasedBackendValidation(mEnableBackendValidation);
    instance->EnableBeginCaptureOnStartup(mBeginCaptureOnStartup);

    instance->DiscoverDefaultAdapters();

#ifdef DAWN_ENABLE_BACKEND_OPENGL
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

    glfwMakeContextCurrent(mOpenGLWindow);
    dawn_native::opengl::AdapterDiscoveryOptions adapterOptions;
    adapterOptions.getProc = reinterpret_cast<void* (*)(const char*)>(glfwGetProcAddress);
    instance->DiscoverAdapters(&adapterOptions);

    glfwDefaultWindowHints();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
    glfwWindowHint(GLFW_CONTEXT_CREATION_API, GLFW_EGL_CONTEXT_API);
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);

    mOpenGLESWindow = glfwCreateWindow(400, 400, "Dawn OpenGLES test window", nullptr, nullptr);

    glfwMakeContextCurrent(mOpenGLESWindow);
    dawn_native::opengl::AdapterDiscoveryOptionsES adapterOptionsES;
    adapterOptionsES.getProc = adapterOptions.getProc;
    instance->DiscoverAdapters(&adapterOptionsES);
    glfwWindowHint(GLFW_VISIBLE, GLFW_TRUE);
#endif  // DAWN_ENABLE_BACKEND_OPENGL

    return instance;
}

GLFWwindow* DawnTestEnvironment::GetOpenGLWindow() const {
    return mOpenGLWindow;
}

GLFWwindow* DawnTestEnvironment::GetOpenGLESWindow() const {
    return mOpenGLESWindow;
}

void DawnTestEnvironment::SelectPreferredAdapterProperties(const dawn_native::Instance* instance) {
    // Get the first available preferred device type.
    dawn_native::DeviceType preferredDeviceType = static_cast<dawn_native::DeviceType>(-1);
    bool hasDevicePreference = false;
    for (dawn_native::DeviceType devicePreference : mDevicePreferences) {
        for (const dawn_native::Adapter& adapter : instance->GetAdapters()) {
            wgpu::AdapterProperties properties;
            adapter.GetProperties(&properties);

            if (adapter.GetDeviceType() == devicePreference) {
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
    for (const dawn_native::Adapter& adapter : instance->GetAdapters()) {
        wgpu::AdapterProperties properties;
        adapter.GetProperties(&properties);

        // The adapter is selected if:
        bool selected = false;
        if (mHasVendorIdFilter) {
            // It matches the vendor id, if present.
            selected = mVendorIdFilter == properties.vendorID;

            if (!mDevicePreferences.empty()) {
                dawn::WarningLog() << "Vendor ID filter provided. Ignoring device type preference.";
            }
        } else if (hasDevicePreference) {
            // There is a device preference and:
            selected =
                // The device type matches the first available preferred type for that backend, if
                // present.
                (adapter.GetDeviceType() == preferredDeviceType) ||
                // Always select Unknown OpenGL adapters if we don't want a CPU adapter.
                // OpenGL will usually be unknown because we can't query the device type.
                // If we ever have Swiftshader GL (unlikely), we could set the DeviceType properly.
                (preferredDeviceType != dawn_native::DeviceType::CPU &&
                 adapter.GetDeviceType() == dawn_native::DeviceType::Unknown &&
                 properties.backendType == wgpu::BackendType::OpenGL) ||
                // Always select the Null backend. There are few tests on this backend, and they run
                // quickly. This is temporary as to not lose coverage. We can group it with
                // Swiftshader as a CPU adapter when we have Swiftshader tests.
                (properties.backendType == wgpu::BackendType::Null);
        } else {
            // No vendor id or device preference was provided (select all).
            selected = true;
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

                // HACK: This is a hack to get Tint generator enabled on all tests
                // without adding a new test suite in Chromium's infra config but skipping
                // that suite on all unsupported platforms. Once we have basic functionality and
                // test skips on all backends, we can remove this and use a test suite with
                // use_tint_generator in the command line args instead.
                if (params[i].backendType == wgpu::BackendType::Vulkan ||
                    params[i].backendType == wgpu::BackendType::OpenGL ||
                    params[i].backendType == wgpu::BackendType::OpenGLES) {
                    BackendTestConfig configWithTint = params[i];
                    configWithTint.forceEnabledWorkarounds.push_back("use_tint_generator");
                    testParams.push_back(AdapterTestParam(configWithTint, adapterProperties));
                }
            }
        }
    }
    return testParams;
}

void DawnTestEnvironment::PrintTestConfigurationAndAdapterInfo(
    dawn_native::Instance* instance) const {
    dawn::LogMessage log = dawn::InfoLog();
    log << "Testing configuration\n"
           "---------------------\n"
           "UseWire: "
        << (mUseWire ? "true" : "false")
        << "\n"
           "EnableBackendValidation: "
        << (mEnableBackendValidation ? "true" : "false");

    if (GetEnabledToggles().size() > 0) {
        log << "\n"
               "Enabled Toggles\n";
        for (const std::string& toggle : GetEnabledToggles()) {
            const dawn_native::ToggleInfo* info = instance->GetToggleInfo(toggle.c_str());
            ASSERT(info != nullptr);
            log << " - " << info->name << ": " << info->description << "\n";
        }
    }

    if (GetDisabledToggles().size() > 0) {
        log << "\n"
               "Disabled Toggles\n";
        for (const std::string& toggle : GetDisabledToggles()) {
            const dawn_native::ToggleInfo* info = instance->GetToggleInfo(toggle.c_str());
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

bool DawnTestEnvironment::IsBackendValidationEnabled() const {
    return mEnableBackendValidation;
}

dawn_native::Instance* DawnTestEnvironment::GetInstance() const {
    return mInstance.get();
}

bool DawnTestEnvironment::HasVendorIdFilter() const {
    return mHasVendorIdFilter;
}

uint32_t DawnTestEnvironment::GetVendorIdFilter() const {
    return mVendorIdFilter;
}

const char* DawnTestEnvironment::GetWireTraceDir() const {
    if (mWireTraceDir.length() == 0) {
        return nullptr;
    }
    return mWireTraceDir.c_str();
}

const std::vector<std::string>& DawnTestEnvironment::GetEnabledToggles() const {
    return mEnabledToggles;
}

const std::vector<std::string>& DawnTestEnvironment::GetDisabledToggles() const {
    return mDisabledToggles;
}

class WireServerTraceLayer : public dawn_wire::CommandHandler {
  public:
    WireServerTraceLayer(const char* file, dawn_wire::CommandHandler* handler)
        : dawn_wire::CommandHandler(), mHandler(handler) {
        mFile.open(file, std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);
    }

    const volatile char* HandleCommands(const volatile char* commands, size_t size) override {
        mFile.write(const_cast<const char*>(commands), size);
        return mHandler->HandleCommands(commands, size);
    }

  private:
    dawn_wire::CommandHandler* mHandler;
    std::ofstream mFile;
};

// Implementation of DawnTest

DawnTestBase::DawnTestBase(const AdapterTestParam& param) : mParam(param) {
}

DawnTestBase::~DawnTestBase() {
    // We need to destroy child objects before the Device
    mReadbackSlots.clear();
    queue = wgpu::Queue();
    device = wgpu::Device();

    mWireClient = nullptr;
    mWireServer = nullptr;
    if (gTestEnv->UsesWire()) {
        backendProcs.deviceRelease(backendDevice);
    }

    dawnProcSetProcs(nullptr);
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

bool DawnTestBase::IsMacOS() const {
#ifdef DAWN_PLATFORM_APPLE
    return true;
#else
    return false;
#endif
}

bool DawnTestBase::UsesWire() const {
    return gTestEnv->UsesWire();
}

bool DawnTestBase::IsBackendValidationEnabled() const {
    return gTestEnv->IsBackendValidationEnabled();
}

bool DawnTestBase::HasWGSL() const {
#ifdef DAWN_ENABLE_WGSL
    return true;
#else
    return false;
#endif
}

bool DawnTestBase::IsAsan() const {
#if defined(ADDRESS_SANITIZER)
    return true;
#else
    return false;
#endif
}

bool DawnTestBase::HasToggleEnabled(const char* toggle) const {
    auto toggles = dawn_native::GetTogglesUsed(backendDevice);
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

wgpu::Instance DawnTestBase::GetInstance() const {
    return gTestEnv->GetInstance()->Get();
}

dawn_native::Adapter DawnTestBase::GetAdapter() const {
    return mBackendAdapter;
}

std::vector<const char*> DawnTestBase::GetRequiredExtensions() {
    return {};
}

const wgpu::AdapterProperties& DawnTestBase::GetAdapterProperties() const {
    return mParam.adapterProperties;
}

bool DawnTestBase::SupportsExtensions(const std::vector<const char*>& extensions) {
    ASSERT(mBackendAdapter);
    std::set<std::string> supportedExtensionsSet;
    for (const char* supportedExtensionName : mBackendAdapter.GetSupportedExtensions()) {
        supportedExtensionsSet.insert(supportedExtensionName);
    }

    for (const char* extensionName : extensions) {
        if (supportedExtensionsSet.find(extensionName) == supportedExtensionsSet.end()) {
            return false;
        }
    }

    return true;
}

void DawnTestBase::SetUp() {
    {
        // Find the adapter that exactly matches our adapter properties.
        const auto& adapters = gTestEnv->GetInstance()->GetAdapters();
        const auto& it = std::find_if(
            adapters.begin(), adapters.end(), [&](const dawn_native::Adapter& adapter) {
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

    // Setup the per-test platform. Tests can provide one by overloading CreateTestPlatform.
    mTestPlatform = CreateTestPlatform();
    gTestEnv->GetInstance()->SetPlatform(mTestPlatform.get());

    // Create the device from the adapter
    for (const char* forceEnabledWorkaround : mParam.forceEnabledWorkarounds) {
        ASSERT(gTestEnv->GetInstance()->GetToggleInfo(forceEnabledWorkaround) != nullptr);
    }
    for (const char* forceDisabledWorkaround : mParam.forceDisabledWorkarounds) {
        ASSERT(gTestEnv->GetInstance()->GetToggleInfo(forceDisabledWorkaround) != nullptr);
    }
    dawn_native::DeviceDescriptor deviceDescriptor;
    deviceDescriptor.forceEnabledToggles = mParam.forceEnabledWorkarounds;
    deviceDescriptor.forceDisabledToggles = mParam.forceDisabledWorkarounds;
    deviceDescriptor.requiredExtensions = GetRequiredExtensions();

    for (const std::string& toggle : gTestEnv->GetEnabledToggles()) {
        const dawn_native::ToggleInfo* info =
            gTestEnv->GetInstance()->GetToggleInfo(toggle.c_str());
        ASSERT(info != nullptr);
        deviceDescriptor.forceEnabledToggles.push_back(info->name);
    }

    for (const std::string& toggle : gTestEnv->GetDisabledToggles()) {
        const dawn_native::ToggleInfo* info =
            gTestEnv->GetInstance()->GetToggleInfo(toggle.c_str());
        ASSERT(info != nullptr);
        deviceDescriptor.forceDisabledToggles.push_back(info->name);
    }

    backendDevice = mBackendAdapter.CreateDevice(&deviceDescriptor);
    ASSERT_NE(nullptr, backendDevice);

    backendProcs = dawn_native::GetProcs();

    // Choose whether to use the backend procs and devices directly, or set up the wire.
    WGPUDevice cDevice = nullptr;
    DawnProcTable procs;

    if (gTestEnv->UsesWire()) {
        mC2sBuf = std::make_unique<utils::TerribleCommandBuffer>();
        mS2cBuf = std::make_unique<utils::TerribleCommandBuffer>();

        dawn_wire::WireServerDescriptor serverDesc = {};
        serverDesc.device = backendDevice;
        serverDesc.procs = &backendProcs;
        serverDesc.serializer = mS2cBuf.get();

        mWireServer.reset(new dawn_wire::WireServer(serverDesc));
        mC2sBuf->SetHandler(mWireServer.get());

        if (gTestEnv->GetWireTraceDir() != nullptr) {
            std::string file =
                std::string(
                    ::testing::UnitTest::GetInstance()->current_test_info()->test_suite_name()) +
                "_" + ::testing::UnitTest::GetInstance()->current_test_info()->name();
            // Replace slashes in gtest names with underscores so everything is in one directory.
            std::replace(file.begin(), file.end(), '/', '_');

            std::string fullPath = gTestEnv->GetWireTraceDir() + file;

            mWireServerTraceLayer.reset(
                new WireServerTraceLayer(fullPath.c_str(), mWireServer.get()));
            mC2sBuf->SetHandler(mWireServerTraceLayer.get());
        }

        dawn_wire::WireClientDescriptor clientDesc = {};
        clientDesc.serializer = mC2sBuf.get();

        mWireClient.reset(new dawn_wire::WireClient(clientDesc));
        cDevice = mWireClient->GetDevice();
        procs = dawn_wire::client::GetProcs();
        mS2cBuf->SetHandler(mWireClient.get());
    } else {
        procs = backendProcs;
        cDevice = backendDevice;
    }

    // Set up the device and queue because all tests need them, and DawnTestBase needs them too for
    // the deferred expectations.
    dawnProcSetProcs(&procs);
    device = wgpu::Device::Acquire(cDevice);
    queue = device.GetDefaultQueue();

    device.SetUncapturedErrorCallback(OnDeviceError, this);
    device.SetDeviceLostCallback(OnDeviceLost, this);
#if defined(DAWN_ENABLE_BACKEND_OPENGL)
    if (IsOpenGL()) {
        glfwMakeContextCurrent(gTestEnv->GetOpenGLWindow());
    } else if (IsOpenGLES()) {
        glfwMakeContextCurrent(gTestEnv->GetOpenGLESWindow());
    }
#endif

    // A very large number of tests hang on Intel D3D12 with the debug adapter after a driver
    // upgrade. Violently suppress this whole configuration until we figure out what to do.
    // See https://crbug.com/dawn/598
    DAWN_SKIP_TEST_IF(IsBackendValidationEnabled() && IsIntel() && IsD3D12());
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
                  dawn_native::GetDeprecationWarningCountForTesting(device.Get()));
    }
}

void DawnTestBase::StartExpectDeviceError() {
    mExpectError = true;
    mError = false;
}
bool DawnTestBase::EndExpectDeviceError() {
    mExpectError = false;
    return mError;
}

// static
void DawnTestBase::OnDeviceError(WGPUErrorType type, const char* message, void* userdata) {
    ASSERT(type != WGPUErrorType_NoError);
    DawnTestBase* self = static_cast<DawnTestBase*>(userdata);

    ASSERT_TRUE(self->mExpectError) << "Got unexpected device error: " << message;
    ASSERT_FALSE(self->mError) << "Got two errors in expect block";
    self->mError = true;
}

void DawnTestBase::OnDeviceLost(const char* message, void* userdata) {
    // Using ADD_FAILURE + ASSERT instead of FAIL to prevent the current test from continuing with a
    // corrupt state.
    ADD_FAILURE() << "Device Lost during test: " << message;
    ASSERT(false);
}

std::ostringstream& DawnTestBase::AddBufferExpectation(const char* file,
                                                       int line,
                                                       const wgpu::Buffer& buffer,
                                                       uint64_t offset,
                                                       uint64_t size,
                                                       detail::Expectation* expectation) {
    auto readback = ReserveReadback(size);

    // We need to enqueue the copy immediately because by the time we resolve the expectation,
    // the buffer might have been modified.
    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    encoder.CopyBufferToBuffer(buffer, offset, readback.buffer, readback.offset, size);

    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    DeferredExpectation deferred;
    deferred.file = file;
    deferred.line = line;
    deferred.readbackSlot = readback.slot;
    deferred.readbackOffset = readback.offset;
    deferred.size = size;
    deferred.rowBytes = size;
    deferred.bytesPerRow = size;
    deferred.expectation.reset(expectation);

    mDeferredExpectations.push_back(std::move(deferred));
    mDeferredExpectations.back().message = std::make_unique<std::ostringstream>();
    return *(mDeferredExpectations.back().message.get());
}

std::ostringstream& DawnTestBase::AddTextureExpectationImpl(const char* file,
                                                            int line,
                                                            detail::Expectation* expectation,
                                                            const wgpu::Texture& texture,
                                                            uint32_t x,
                                                            uint32_t y,
                                                            uint32_t width,
                                                            uint32_t height,
                                                            uint32_t level,
                                                            uint32_t slice,
                                                            wgpu::TextureAspect aspect,
                                                            uint32_t dataSize,
                                                            uint32_t bytesPerRow) {
    if (bytesPerRow == 0) {
        bytesPerRow = Align(width * dataSize, kTextureBytesPerRowAlignment);
    } else {
        ASSERT(bytesPerRow >= width * dataSize);
        ASSERT(bytesPerRow == Align(bytesPerRow, kTextureBytesPerRowAlignment));
    }

    uint32_t rowsPerImage = height;
    uint32_t depth = 1;
    uint32_t size =
        utils::RequiredBytesInCopy(bytesPerRow, rowsPerImage, width, height, depth, dataSize);

    // TODO(enga): We should have the map async alignment in Contants.h. Also, it should change to 8
    // for Float64Array.
    auto readback = ReserveReadback(Align(size, 4));

    // We need to enqueue the copy immediately because by the time we resolve the expectation,
    // the texture might have been modified.
    wgpu::TextureCopyView textureCopyView =
        utils::CreateTextureCopyView(texture, level, {x, y, slice}, aspect);
    wgpu::BufferCopyView bufferCopyView =
        utils::CreateBufferCopyView(readback.buffer, readback.offset, bytesPerRow, rowsPerImage);
    wgpu::Extent3D copySize = {width, height, 1};

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    encoder.CopyTextureToBuffer(&textureCopyView, &bufferCopyView, &copySize);

    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    DeferredExpectation deferred;
    deferred.file = file;
    deferred.line = line;
    deferred.readbackSlot = readback.slot;
    deferred.readbackOffset = readback.offset;
    deferred.size = size;
    deferred.rowBytes = width * dataSize;
    deferred.bytesPerRow = bytesPerRow;
    deferred.expectation.reset(expectation);

    mDeferredExpectations.push_back(std::move(deferred));
    mDeferredExpectations.back().message = std::make_unique<std::ostringstream>();
    return *(mDeferredExpectations.back().message.get());
}

void DawnTestBase::WaitABit() {
    device.Tick();
    FlushWire();

    utils::USleep(100);
}

void DawnTestBase::FlushWire() {
    if (gTestEnv->UsesWire()) {
        bool C2SFlushed = mC2sBuf->Flush();
        bool S2CFlushed = mS2cBuf->Flush();
        ASSERT(C2SFlushed);
        ASSERT(S2CFlushed);
    }
}

void DawnTestBase::WaitForAllOperations() {
    wgpu::Queue queue = device.GetDefaultQueue();
    wgpu::Fence fence = queue.CreateFence();

    // Force the currently submitted operations to completed.
    queue.Signal(fence, 1);
    while (fence.GetCompletedValue() < 1) {
        WaitABit();
    }
}

DawnTestBase::ReadbackReservation DawnTestBase::ReserveReadback(uint64_t readbackSize) {
    // For now create a new MapRead buffer for each readback
    // TODO(cwallez@chromium.org): eventually make bigger buffers and allocate linearly?
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
        slot.buffer.MapAsync(wgpu::MapMode::Read, 0, 0, SlotMapCallback, userdata);
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

std::unique_ptr<dawn_platform::Platform> DawnTestBase::CreateTestPlatform() {
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

    template <typename T>
    ExpectEq<T>::ExpectEq(T singleValue) {
        mExpected.push_back(singleValue);
    }

    template <typename T>
    ExpectEq<T>::ExpectEq(const T* values, const unsigned int count) {
        mExpected.assign(values, values + count);
    }

    template <typename T>
    testing::AssertionResult ExpectEq<T>::Check(const void* data, size_t size) {
        DAWN_ASSERT(size == sizeof(T) * mExpected.size());

        const T* actual = static_cast<const T*>(data);

        for (size_t i = 0; i < mExpected.size(); ++i) {
            if (actual[i] != mExpected[i]) {
                testing::AssertionResult result = testing::AssertionFailure()
                                                  << "Expected data[" << i << "] to be "
                                                  << mExpected[i] << ", actual " << actual[i]
                                                  << std::endl;

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

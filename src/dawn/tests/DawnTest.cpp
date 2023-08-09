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
#include <atomic>
#include <fstream>
#include <iomanip>
#include <regex>
#include <set>
#include <sstream>
#include <tuple>
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

namespace dawn {
namespace {

struct MapReadUserdata {
    DawnTestBase* test;
    size_t slot;
};

DawnTestEnvironment* gTestEnv = nullptr;
DawnTestBase* gCurrentTest = nullptr;

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

// A helper class to create DawnTogglesDescriptor from test params
struct ParamTogglesHelper {
    std::vector<const char*> enabledToggles;
    std::vector<const char*> disabledToggles;
    wgpu::DawnTogglesDescriptor togglesDesc;

    // Create toggles descriptor for a given stage from test param and global test env
    ParamTogglesHelper(const AdapterTestParam& testParam, native::ToggleStage requiredStage) {
        for (const char* requireEnabledWorkaround : testParam.forceEnabledWorkarounds) {
            const native::ToggleInfo* info =
                gTestEnv->GetInstance()->GetToggleInfo(requireEnabledWorkaround);
            ASSERT(info != nullptr);
            if (info->stage == requiredStage) {
                enabledToggles.push_back(requireEnabledWorkaround);
            }
        }
        for (const char* requireDisabledWorkaround : testParam.forceDisabledWorkarounds) {
            const native::ToggleInfo* info =
                gTestEnv->GetInstance()->GetToggleInfo(requireDisabledWorkaround);
            ASSERT(info != nullptr);
            if (info->stage == requiredStage) {
                disabledToggles.push_back(requireDisabledWorkaround);
            }
        }

        for (const std::string& toggle : gTestEnv->GetEnabledToggles()) {
            const native::ToggleInfo* info = gTestEnv->GetInstance()->GetToggleInfo(toggle.c_str());
            ASSERT(info != nullptr);
            if (info->stage == requiredStage) {
                enabledToggles.push_back(info->name);
            }
        }

        for (const std::string& toggle : gTestEnv->GetDisabledToggles()) {
            const native::ToggleInfo* info = gTestEnv->GetInstance()->GetToggleInfo(toggle.c_str());
            ASSERT(info != nullptr);
            if (info->stage == requiredStage) {
                disabledToggles.push_back(info->name);
            }
        }

        togglesDesc = {};
        togglesDesc.enabledToggles = enabledToggles.data();
        togglesDesc.enabledTogglesCount = enabledToggles.size();
        togglesDesc.disabledToggles = disabledToggles.data();
        togglesDesc.disabledTogglesCount = disabledToggles.size();
    }
};
}  // anonymous namespace

DawnTestBase::PrintToStringParamName::PrintToStringParamName(const char* test) : mTest(test) {}

std::string DawnTestBase::PrintToStringParamName::SanitizeParamName(
    std::string paramName,
    const TestAdapterProperties& properties,
    size_t index) const {
    // Sanitize the adapter name for GoogleTest
    std::string sanitizedName = std::regex_replace(paramName, std::regex("[^a-zA-Z0-9]+"), "_") +
                                (properties.compatibilityMode ? "_compat" : "");

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

}  // namespace dawn

void InitDawnEnd2EndTestEnvironment(int argc, char** argv) {
    dawn::gTestEnv = new dawn::DawnTestEnvironment(argc, argv);
    testing::AddGlobalTestEnvironment(dawn::gTestEnv);
}

namespace dawn {

// Implementation of DawnTestEnvironment

// static
void DawnTestEnvironment::SetEnvironment(DawnTestEnvironment* env) {
    gTestEnv = env;
}

DawnTestEnvironment::DawnTestEnvironment(int argc, char** argv) {
    ParseArgs(argc, argv);

    if (mBackendValidationLevel != native::BackendValidationLevel::Disabled) {
        mPlatformDebugLogger =
            std::unique_ptr<utils::PlatformDebugLogger>(utils::CreatePlatformDebugLogger());
    }

    // Create a temporary instance to select available and preferred adapters. This is done before
    // test instantiation so GetAvailableAdapterTestParamsForBackends can generate test
    // parameterizations all selected adapters. We drop the instance at the end of this function
    // because the Vulkan validation layers use static global mutexes which behave badly when
    // Chromium's test launcher forks the test process. The instance will be recreated on test
    // environment setup.
    std::unique_ptr<native::Instance> instance = CreateInstance();
    ASSERT(instance);

    if (!ValidateToggles(instance.get())) {
        return;
    }

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

        if (strcmp("-s", argv[i]) == 0 || strcmp("--enable-implicit-device-sync", argv[i]) == 0) {
            mEnableImplicitDeviceSync = true;
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
                    mBackendValidationLevel = native::BackendValidationLevel::Full;
                } else if (strcmp(level, "=partial") == 0) {
                    mBackendValidationLevel = native::BackendValidationLevel::Partial;
                } else if (strcmp(level, "=disabled") == 0) {
                    mBackendValidationLevel = native::BackendValidationLevel::Disabled;
                } else {
                    ErrorLog() << "Invalid backend validation level" << level;
                    UNREACHABLE();
                }
            } else {
                mBackendValidationLevel = native::BackendValidationLevel::Partial;
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

        constexpr const char kUseAngleArg[] = "--use-angle=";
        argLen = sizeof(kUseAngleArg) - 1;
        if (strncmp(argv[i], kUseAngleArg, argLen) == 0) {
            mANGLEBackend = argv[i] + argLen;
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
                        ErrorLog() << "Invalid device type preference: " << type;
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
            if (strcmp("d3d11", param) == 0) {
                mBackendTypeFilter = wgpu::BackendType::D3D11;
            } else if (strcmp("d3d12", param) == 0) {
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
                ErrorLog()
                    << "Invalid backend \"" << param
                    << "\". Valid backends are: d3d12, metal, null, opengl, opengles, vulkan.";
                UNREACHABLE();
            }
            mHasBackendTypeFilter = true;
            continue;
        }
        if (strcmp("-h", argv[i]) == 0 || strcmp("--help", argv[i]) == 0) {
            InfoLog()
                << "\n\nUsage: " << argv[0]
                << " [GTEST_FLAGS...] [-w] [-c]\n"
                   "    [--enable-toggles=toggles] [--disable-toggles=toggles]\n"
                   "    [--backend=x]\n"
                   "    [--adapter-vendor-id=x] "
                   "[--enable-backend-validation[=full,partial,disabled]]\n"
                   "    [--exclusive-device-type-preference=integrated,cpu,discrete]\n\n"
                   "  -w, --use-wire: Run the tests through the wire (defaults to no wire)\n"
                   "  -s, --enable-implicit-device-sync: Run the tests with implicit device "
                   "synchronization feature (defaults to false)\n"
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

        WarningLog() << " Unused argument: " << argv[i];
    }

    // TODO(crbug.com/dawn/1678): DawnWire doesn't support thread safe API yet.
    if (mUseWire && mEnableImplicitDeviceSync) {
        ErrorLog()
            << "--use-wire and --enable-implicit-device-sync cannot be used at the same time";
        UNREACHABLE();
    }
}

std::unique_ptr<native::Instance> DawnTestEnvironment::CreateInstance(
    platform::Platform* platform) {
    // Create an instance with toggle AllowUnsafeAPIs enabled, which would be inherited to
    // adapter and device toggles and allow us to test unsafe apis (including experimental
    // features).
    const char* allowUnsafeApisToggle = "allow_unsafe_apis";
    wgpu::DawnTogglesDescriptor instanceToggles;
    instanceToggles.enabledTogglesCount = 1;
    instanceToggles.enabledToggles = &allowUnsafeApisToggle;

    wgpu::DawnInstanceDescriptor dawnInstanceDesc;
    dawnInstanceDesc.platform = platform;
    dawnInstanceDesc.nextInChain = &instanceToggles;

    wgpu::InstanceDescriptor instanceDesc;
    instanceDesc.nextInChain = &dawnInstanceDesc;

    auto instance = std::make_unique<native::Instance>(
        reinterpret_cast<const WGPUInstanceDescriptor*>(&instanceDesc));
    instance->EnableBeginCaptureOnStartup(mBeginCaptureOnStartup);
    instance->SetBackendValidationLevel(mBackendValidationLevel);
    instance->EnableAdapterBlocklist(false);

#ifdef DAWN_ENABLE_BACKEND_OPENGLES
    if (GetEnvironmentVar("ANGLE_DEFAULT_PLATFORM").first.empty()) {
        const char* anglePlatform;
        if (!mANGLEBackend.empty()) {
            anglePlatform = mANGLEBackend.c_str();
        } else {
#if DAWN_PLATFORM_IS(WINDOWS)
            anglePlatform = "d3d11";
#else
            anglePlatform = "swiftshader";
#endif
        }
        SetEnvironmentVar("ANGLE_DEFAULT_PLATFORM", anglePlatform);
    }
#endif  // DAWN_ENABLE_BACKEND_OPENGLES

    return instance;
}

void DawnTestEnvironment::SelectPreferredAdapterProperties(const native::Instance* instance) {
    dawnProcSetProcs(&dawn::native::GetProcs());

    // Get the first available preferred device type.
    std::optional<wgpu::AdapterType> preferredDeviceType;
    [&] {
        for (wgpu::AdapterType devicePreference : mDevicePreferences) {
            for (bool compatibilityMode : {false, true}) {
                wgpu::RequestAdapterOptions adapterOptions;
                adapterOptions.compatibilityMode = compatibilityMode;
                for (const native::Adapter& adapter :
                     instance->EnumerateAdapters(&adapterOptions)) {
                    wgpu::AdapterProperties properties;
                    adapter.GetProperties(&properties);

                    if (properties.adapterType == devicePreference) {
                        // Found a matching preferred device type. Return to break out of all loops.
                        preferredDeviceType = devicePreference;
                        return;
                    }
                }
            }
        }
    }();

    std::set<std::tuple<wgpu::BackendType, std::string, bool>> adapterNameSet;
    for (bool compatibilityMode : {false, true}) {
        wgpu::RequestAdapterOptions adapterOptions;
        adapterOptions.compatibilityMode = compatibilityMode;
        for (const native::Adapter& adapter : instance->EnumerateAdapters(&adapterOptions)) {
            wgpu::AdapterProperties properties;
            adapter.GetProperties(&properties);

            // Skip non-OpenGLES compat adapters. Metal/Vulkan/D3D12 support
            // core WebGPU.
            // D3D11 is in an experimental state where it may support core.
            // See crbug.com/dawn/1820 for determining d3d11 capabilities.
            if (properties.compatibilityMode &&
                properties.backendType != wgpu::BackendType::OpenGLES) {
                continue;
            }

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
                    WarningLog() << "Vendor ID filter provided. Ignoring device type preference.";
                }
            }
            if (preferredDeviceType) {
                // There is a device preference and:
                selected &=
                    // The device type doesn't match the first available preferred type for that
                    // backend, if present.
                    (properties.adapterType == *preferredDeviceType) ||
                    // Always select Unknown OpenGL adapters if we don't want a CPU adapter.
                    // OpenGL will usually be unknown because we can't query the device type.
                    // If we ever have Swiftshader GL (unlikely), we could set the DeviceType
                    // properly.
                    (*preferredDeviceType != wgpu::AdapterType::CPU &&
                     properties.adapterType == wgpu::AdapterType::Unknown &&
                     (properties.backendType == wgpu::BackendType::OpenGL ||
                      properties.backendType == wgpu::BackendType::OpenGLES)) ||
                    // Always select the Null backend. There are few tests on this backend, and they
                    // run quickly. This is temporary as to not lose coverage. We can group it with
                    // Swiftshader as a CPU adapter when we have Swiftshader tests.
                    (properties.backendType == wgpu::BackendType::Null);
            }

            // In Windows Remote Desktop sessions we may be able to discover multiple adapters that
            // have the same name and backend type. We will just choose one adapter from them in our
            // tests.
            const auto adapterTypeAndName = std::tuple(
                properties.backendType, std::string(properties.name), properties.compatibilityMode);
            if (adapterNameSet.find(adapterTypeAndName) == adapterNameSet.end()) {
                adapterNameSet.insert(adapterTypeAndName);
                mAdapterProperties.emplace_back(properties, selected);
            }
        }
    }
}

std::vector<AdapterTestParam> DawnTestEnvironment::GetAvailableAdapterTestParamsForBackends(
    const BackendTestConfig* params,
    size_t numParams) {
    std::vector<AdapterTestParam> testParams;
    for (size_t i = 0; i < numParams; ++i) {
        const auto& backendTestParams = params[i];
        for (const auto& adapterProperties : mAdapterProperties) {
            if (backendTestParams.backendType == adapterProperties.backendType &&
                adapterProperties.selected) {
                testParams.push_back(AdapterTestParam(backendTestParams, adapterProperties));
            }
        }
    }
    return testParams;
}

bool DawnTestEnvironment::ValidateToggles(native::Instance* instance) const {
    LogMessage err = ErrorLog();
    for (const std::string& toggle : GetEnabledToggles()) {
        if (!instance->GetToggleInfo(toggle.c_str())) {
            err << "unrecognized toggle: '" << toggle << "'\n";
            return false;
        }
    }
    for (const std::string& toggle : GetDisabledToggles()) {
        if (!instance->GetToggleInfo(toggle.c_str())) {
            err << "unrecognized toggle: '" << toggle << "'\n";
            return false;
        }
    }
    return true;
}

void DawnTestEnvironment::PrintTestConfigurationAndAdapterInfo(native::Instance* instance) const {
    LogMessage log = InfoLog();
    log << "Testing configuration\n"
           "---------------------\n"
           "UseWire: "
        << (mUseWire ? "true" : "false")
        << "\n"
           "Implicit device synchronization: "
        << (mEnableImplicitDeviceSync ? "enabled" : "disabled")
        << "\n"
           "Run suppressed tests: "
        << (mRunSuppressedTests ? "true" : "false")
        << "\n"
           "BackendValidation: ";

    switch (mBackendValidationLevel) {
        case native::BackendValidationLevel::Full:
            log << "full";
            break;
        case native::BackendValidationLevel::Partial:
            log << "partial";
            break;
        case native::BackendValidationLevel::Disabled:
            log << "disabled";
            break;
        default:
            UNREACHABLE();
    }

    if (GetEnabledToggles().size() > 0) {
        log << "\n"
               "Enabled Toggles\n";
        for (const std::string& toggle : GetEnabledToggles()) {
            const native::ToggleInfo* info = instance->GetToggleInfo(toggle.c_str());
            ASSERT(info != nullptr);
            log << " - " << info->name << ": " << info->description << "\n";
        }
    }

    if (GetDisabledToggles().size() > 0) {
        log << "\n"
               "Disabled Toggles\n";
        for (const std::string& toggle : GetDisabledToggles()) {
            const native::ToggleInfo* info = instance->GetToggleInfo(toggle.c_str());
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
        log << std::showbase << std::hex << std::setfill('0') << std::setw(4) << " - \""
            << properties.name << "\" - \"" << properties.driverDescription
            << (properties.selected ? " [Selected]" : "") << "\"\n"
            << "   type: " << properties.AdapterTypeName()
            << ", backend: " << properties.ParamName()
            << ", compatibilityMode: " << (properties.compatibilityMode ? "true" : "false") << "\n"
            << "   vendorId: 0x" << vendorId.str() << ", deviceId: 0x" << deviceId.str() << "\n";

        if (!properties.vendorName.empty() || !properties.architecture.empty()) {
            log << "   vendorName: " << properties.vendorName
                << ", architecture: " << properties.architecture << "\n";
        }
    }
}

void DawnTestEnvironment::SetUp() {
    mInstance = CreateInstance();
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

bool DawnTestEnvironment::IsImplicitDeviceSyncEnabled() const {
    return mEnableImplicitDeviceSync;
}

bool DawnTestEnvironment::RunSuppressedTests() const {
    return mRunSuppressedTests;
}

native::BackendValidationLevel DawnTestEnvironment::GetBackendValidationLevel() const {
    return mBackendValidationLevel;
}

native::Instance* DawnTestEnvironment::GetInstance() const {
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

DawnTestBase::DawnTestBase(const AdapterTestParam& param) : mParam(param) {
    gCurrentTest = this;

    DawnProcTable procs = native::GetProcs();
    // Override procs to provide harness-specific behavior to always select the adapter required in
    // testing parameter, and to allow fixture-specific overriding of the test device with
    // CreateDeviceImpl.
    procs.instanceRequestAdapter = [](WGPUInstance instance, const WGPURequestAdapterOptions*,
                                      WGPURequestAdapterCallback callback, void* userdata) {
        ASSERT(gCurrentTest);

        // Use the required toggles of test case when creating adapter.
        const auto& enabledToggles = gCurrentTest->mParam.forceEnabledWorkarounds;
        const auto& disabledToggles = gCurrentTest->mParam.forceDisabledWorkarounds;
        wgpu::DawnTogglesDescriptor adapterToggles;
        adapterToggles.enabledTogglesCount = enabledToggles.size();
        adapterToggles.enabledToggles = enabledToggles.data();
        adapterToggles.disabledTogglesCount = disabledToggles.size();
        adapterToggles.disabledToggles = disabledToggles.data();

        wgpu::RequestAdapterOptions adapterOptions;
        adapterOptions.nextInChain = &adapterToggles;
        adapterOptions.backendType = gCurrentTest->mParam.adapterProperties.backendType;
        adapterOptions.compatibilityMode = gCurrentTest->mParam.adapterProperties.compatibilityMode;

        // Find the adapter that exactly matches our adapter properties.
        const auto& adapters = gTestEnv->GetInstance()->EnumerateAdapters(&adapterOptions);
        const auto& it =
            std::find_if(adapters.begin(), adapters.end(), [&](const native::Adapter& adapter) {
                wgpu::AdapterProperties properties;
                adapter.GetProperties(&properties);

                const auto& param = gCurrentTest->mParam;
                return (param.adapterProperties.selected &&
                        properties.deviceID == param.adapterProperties.deviceID &&
                        properties.vendorID == param.adapterProperties.vendorID &&
                        properties.adapterType == param.adapterProperties.adapterType &&
                        strcmp(properties.name, param.adapterProperties.name.c_str()) == 0);
            });
        ASSERT(it != adapters.end());
        gCurrentTest->mBackendAdapter = *it;

        WGPUAdapter cAdapter = it->Get();
        ASSERT(cAdapter);
        native::GetProcs().adapterReference(cAdapter);
        callback(WGPURequestAdapterStatus_Success, cAdapter, nullptr, userdata);
    };

    procs.adapterRequestDevice = [](WGPUAdapter adapter, const WGPUDeviceDescriptor* descriptor,
                                    WGPURequestDeviceCallback callback, void* userdata) {
        ASSERT(gCurrentTest);

        // Isolation keys may be enqueued by CreateDevice(std::string isolationKey).
        // CreateDevice calls requestAdapter, so consume them there and forward them
        // to CreateDeviceImpl.
        std::string isolationKey;
        if (!gCurrentTest->mNextIsolationKeyQueue.empty()) {
            isolationKey = std::move(gCurrentTest->mNextIsolationKeyQueue.front());
            gCurrentTest->mNextIsolationKeyQueue.pop();
        }
        WGPUDevice cDevice = gCurrentTest->CreateDeviceImpl(std::move(isolationKey), descriptor);
        ASSERT(cDevice != nullptr);

        gCurrentTest->mLastCreatedBackendDevice = cDevice;
        callback(WGPURequestDeviceStatus_Success, cDevice, nullptr, userdata);
    };

    mWireHelper = utils::CreateWireHelper(procs, gTestEnv->UsesWire(), gTestEnv->GetWireTraceDir());
}

DawnTestBase::~DawnTestBase() {
    mReadbackSlots.clear();
    queue = nullptr;
    device = nullptr;
    mAdapter = nullptr;
    mInstance = nullptr;

    // D3D11 and D3D12's GPU-based validation will accumulate objects over time if the backend
    // device is not destroyed and recreated, so we reset it here.
    if ((IsD3D11() || IsD3D12()) && IsBackendValidationEnabled()) {
        mBackendAdapter.ResetInternalDeviceForTesting();
    }
    mWireHelper.reset();

    // Check that all devices were destructed.
    EXPECT_EQ(gTestEnv->GetInstance()->GetDeviceCountForTesting(), 0u);

    // Unsets the platform since we are cleaning the per-test platform up with the test case.
    native::FromAPI(gTestEnv->GetInstance()->Get())->SetPlatformForTesting(nullptr);

    gCurrentTest = nullptr;
}

bool DawnTestBase::IsD3D11() const {
    return mParam.adapterProperties.backendType == wgpu::BackendType::D3D11;
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

bool DawnTestBase::IsApple() const {
    return gpu_info::IsApple(mParam.adapterProperties.vendorID);
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
    return gpu_info::IsGoogleSwiftshader(mParam.adapterProperties.vendorID,
                                         mParam.adapterProperties.deviceID);
}

bool DawnTestBase::IsANGLE() const {
    return !mParam.adapterProperties.name.find("ANGLE");
}

bool DawnTestBase::IsANGLESwiftShader() const {
    return !mParam.adapterProperties.name.find("ANGLE") &&
           (mParam.adapterProperties.name.find("SwiftShader") != std::string::npos);
}

bool DawnTestBase::IsWARP() const {
    return gpu_info::IsMicrosoftWARP(mParam.adapterProperties.vendorID,
                                     mParam.adapterProperties.deviceID);
}

bool DawnTestBase::IsIntelGen9() const {
    return gpu_info::IsIntelGen9(mParam.adapterProperties.vendorID,
                                 mParam.adapterProperties.deviceID);
}

bool DawnTestBase::IsIntelGen12() const {
    return gpu_info::IsIntelGen12LP(mParam.adapterProperties.vendorID,
                                    mParam.adapterProperties.deviceID) ||
           gpu_info::IsIntelGen12HP(mParam.adapterProperties.vendorID,
                                    mParam.adapterProperties.deviceID);
}

bool DawnTestBase::IsWindows() const {
#if DAWN_PLATFORM_IS(WINDOWS)
    return true;
#else
    return false;
#endif
}

bool DawnTestBase::IsLinux() const {
#if DAWN_PLATFORM_IS(LINUX)
    return true;
#else
    return false;
#endif
}

bool DawnTestBase::IsMacOS(int32_t majorVersion, int32_t minorVersion) const {
#if DAWN_PLATFORM_IS(MACOS)
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

bool DawnTestBase::IsAndroid() const {
#if DAWN_PLATFORM_IS(ANDROID)
    return true;
#else
    return false;
#endif
}

bool DawnTestBase::UsesWire() const {
    return gTestEnv->UsesWire();
}

bool DawnTestBase::IsImplicitDeviceSyncEnabled() const {
    return gTestEnv->IsImplicitDeviceSyncEnabled();
}

bool DawnTestBase::IsBackendValidationEnabled() const {
    return gTestEnv->GetBackendValidationLevel() != native::BackendValidationLevel::Disabled;
}

bool DawnTestBase::IsFullBackendValidationEnabled() const {
    return gTestEnv->GetBackendValidationLevel() == native::BackendValidationLevel::Full;
}

bool DawnTestBase::IsCompatibilityMode() const {
    return mParam.adapterProperties.compatibilityMode;
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

bool DawnTestBase::IsTsan() const {
#if defined(THREAD_SANITIZER)
    return true;
#else
    return false;
#endif
}

bool DawnTestBase::HasToggleEnabled(const char* toggle) const {
    auto toggles = native::GetTogglesUsed(backendDevice);
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

native::Adapter DawnTestBase::GetAdapter() const {
    return mBackendAdapter;
}

std::vector<wgpu::FeatureName> DawnTestBase::GetRequiredFeatures() {
    return {};
}

wgpu::RequiredLimits DawnTestBase::GetRequiredLimits(const wgpu::SupportedLimits&) {
    return {};
}

const TestAdapterProperties& DawnTestBase::GetAdapterProperties() const {
    return mParam.adapterProperties;
}

wgpu::SupportedLimits DawnTestBase::GetAdapterLimits() {
    wgpu::SupportedLimits supportedLimits = {};
    mAdapter.GetLimits(&supportedLimits);
    return supportedLimits;
}

wgpu::SupportedLimits DawnTestBase::GetSupportedLimits() {
    wgpu::SupportedLimits supportedLimits = {};
    device.GetLimits(&supportedLimits);
    return supportedLimits;
}

bool DawnTestBase::SupportsFeatures(const std::vector<wgpu::FeatureName>& features) {
    ASSERT(mBackendAdapter);
    std::vector<wgpu::FeatureName> supportedFeatures;
    uint32_t count = native::GetProcs().adapterEnumerateFeatures(mBackendAdapter.Get(), nullptr);
    supportedFeatures.resize(count);
    native::GetProcs().adapterEnumerateFeatures(
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

void* DawnTestBase::GetUniqueUserdata() {
    return reinterpret_cast<void*>(++mNextUniqueUserdata);
}

WGPUDevice DawnTestBase::CreateDeviceImpl(std::string isolationKey,
                                          const WGPUDeviceDescriptor* descriptor) {
    // Create the device from the adapter
    std::vector<wgpu::FeatureName> requiredFeatures = GetRequiredFeatures();
    if (IsImplicitDeviceSyncEnabled()) {
        requiredFeatures.push_back(wgpu::FeatureName::ImplicitDeviceSynchronization);
    }

    wgpu::SupportedLimits supportedLimits;
    mBackendAdapter.GetLimits(reinterpret_cast<WGPUSupportedLimits*>(&supportedLimits));
    wgpu::RequiredLimits requiredLimits = GetRequiredLimits(supportedLimits);

    wgpu::DeviceDescriptor deviceDescriptor =
        *reinterpret_cast<const wgpu::DeviceDescriptor*>(descriptor);
    deviceDescriptor.requiredLimits = &requiredLimits;
    deviceDescriptor.requiredFeatures = requiredFeatures.data();
    deviceDescriptor.requiredFeaturesCount = requiredFeatures.size();

    wgpu::DawnCacheDeviceDescriptor cacheDesc = {};
    deviceDescriptor.nextInChain = &cacheDesc;
    cacheDesc.isolationKey = isolationKey.c_str();

    // Note that AllowUnsafeAPIs is enabled when creating testing instance and would be
    // inherited to all adapters' toggles set.
    ParamTogglesHelper deviceTogglesHelper(mParam, native::ToggleStage::Device);
    cacheDesc.nextInChain = &deviceTogglesHelper.togglesDesc;

    return mBackendAdapter.CreateDevice(&deviceDescriptor);
}

wgpu::Device DawnTestBase::CreateDevice(std::string isolationKey) {
    wgpu::Device apiDevice;

    // The isolation key will be consumed inside adapterRequestDevice and passed
    // to CreateDeviceImpl.
    mNextIsolationKeyQueue.push(std::move(isolationKey));

    // RequestDevice is overriden by CreateDeviceImpl and device descriptor is ignored by it.
    // Give an empty descriptor.
    // TODO(dawn:1684): Replace empty DeviceDescriptor with nullptr after Dawn wire support it.
    wgpu::DeviceDescriptor deviceDesc = {};

    // Set up the mocks for device loss.
    void* deviceUserdata = GetUniqueUserdata();
    deviceDesc.deviceLostCallback = mDeviceLostCallback.Callback();
    deviceDesc.deviceLostUserdata = mDeviceLostCallback.MakeUserdata(deviceUserdata);

    mAdapter.RequestDevice(
        &deviceDesc,
        [](WGPURequestDeviceStatus, WGPUDevice cDevice, const char*, void* userdata) {
            *static_cast<wgpu::Device*>(userdata) = wgpu::Device::Acquire(cDevice);
        },
        &apiDevice);
    FlushWire();
    ASSERT(apiDevice);

    // Set up the mocks for uncaptured errors.
    apiDevice.SetUncapturedErrorCallback(mDeviceErrorCallback.Callback(),
                                         mDeviceErrorCallback.MakeUserdata(apiDevice.Get()));

    // The loss of the device is expected to happen at the end of the test so at it directly.
    EXPECT_CALL(mDeviceLostCallback,
                Call(WGPUDeviceLostReason_Destroyed, testing::_, deviceUserdata))
        .Times(testing::AtMost(1));

    apiDevice.SetLoggingCallback(
        [](WGPULoggingType type, char const* message, void*) {
            switch (type) {
                case WGPULoggingType_Verbose:
                    DebugLog() << message;
                    break;
                case WGPULoggingType_Warning:
                    WarningLog() << message;
                    break;
                case WGPULoggingType_Error:
                    ErrorLog() << message;
                    break;
                default:
                    InfoLog() << message;
                    break;
            }
        },
        nullptr);

    return apiDevice;
}

void DawnTestBase::SetUp() {
    // Setup the per-test platform. Tests can provide one by overloading CreateTestPlatform.
    // This is NOT a thread-safe operation and is allowed here for testing only.
    mTestPlatform = CreateTestPlatform();
    native::FromAPI(gTestEnv->GetInstance()->Get())->SetPlatformForTesting(mTestPlatform.get());

    mInstance = mWireHelper->RegisterInstance(gTestEnv->GetInstance()->Get());

    std::string traceName =
        std::string(::testing::UnitTest::GetInstance()->current_test_info()->test_suite_name()) +
        "_" + ::testing::UnitTest::GetInstance()->current_test_info()->name();
    mWireHelper->BeginWireTrace(traceName.c_str());

    // RequestAdapter is overriden to ignore RequestAdapterOptions, and select based on test params.
    mInstance.RequestAdapter(
        nullptr,
        [](WGPURequestAdapterStatus, WGPUAdapter cAdapter, const char*, void* userdata) {
            *static_cast<wgpu::Adapter*>(userdata) = wgpu::Adapter::Acquire(cAdapter);
        },
        &mAdapter);
    FlushWire();
    ASSERT(mAdapter);

    device = CreateDevice();
    backendDevice = mLastCreatedBackendDevice;
    ASSERT(backendDevice);
    ASSERT(device);

    queue = device.GetQueue();
}

void DawnTestBase::TearDown() {
    ResolveDeferredExpectationsNow();

    if (!UsesWire() && device) {
        EXPECT_EQ(mLastWarningCount, native::GetDeprecationWarningCountForTesting(device.Get()));
    }
}

void DawnTestBase::DestroyDevice(wgpu::Device deviceToDestroy) {
    wgpu::Device resolvedDevice = deviceToDestroy;
    if (resolvedDevice == nullptr) {
        resolvedDevice = device;
    }

    // No expectation is added because the expectations for this kind of destruction is set up
    // as soon as the device is created.
    resolvedDevice.Destroy();
}

void DawnTestBase::LoseDeviceForTesting(wgpu::Device deviceToLose) {
    wgpu::Device resolvedDevice = deviceToLose;
    if (resolvedDevice == nullptr) {
        resolvedDevice = device;
    }

    EXPECT_CALL(mDeviceLostCallback, Call(WGPUDeviceLostReason_Undefined, testing::_, testing::_))
        .Times(1);
    resolvedDevice.ForceLoss(wgpu::DeviceLostReason::Undefined, "Device lost for testing");
    resolvedDevice.Tick();
}

std::ostringstream& DawnTestBase::AddBufferExpectation(const char* file,
                                                       int line,
                                                       const wgpu::Buffer& buffer,
                                                       uint64_t offset,
                                                       uint64_t size,
                                                       detail::Expectation* expectation) {
    uint64_t alignedSize = Align(size, uint64_t(4));
    auto readback = ReserveReadback(device, alignedSize);

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

    // This expectation might be called from multiple threads
    Mutex::AutoLock lg(&mMutex);

    mDeferredExpectations.push_back(std::move(deferred));
    mDeferredExpectations.back().message = std::make_unique<std::ostringstream>();
    return *(mDeferredExpectations.back().message.get());
}

std::ostringstream& DawnTestBase::AddTextureExpectationImpl(const char* file,
                                                            int line,
                                                            wgpu::Device targetDevice,
                                                            detail::Expectation* expectation,
                                                            const wgpu::Texture& texture,
                                                            wgpu::Origin3D origin,
                                                            wgpu::Extent3D extent,
                                                            uint32_t level,
                                                            wgpu::TextureAspect aspect,
                                                            uint32_t dataSize,
                                                            uint32_t bytesPerRow) {
    ASSERT(targetDevice != nullptr);

    if (bytesPerRow == 0) {
        bytesPerRow = Align(extent.width * dataSize, kTextureBytesPerRowAlignment);
    } else {
        ASSERT(bytesPerRow >= extent.width * dataSize);
        ASSERT(bytesPerRow == Align(bytesPerRow, kTextureBytesPerRowAlignment));
    }

    uint32_t rowsPerImage = extent.height;
    uint32_t size = utils::RequiredBytesInCopy(bytesPerRow, rowsPerImage, extent.width,
                                               extent.height, extent.depthOrArrayLayers, dataSize);

    auto readback = ReserveReadback(targetDevice, Align(size, 4));

    // We need to enqueue the copy immediately because by the time we resolve the expectation,
    // the texture might have been modified.
    wgpu::ImageCopyTexture imageCopyTexture =
        utils::CreateImageCopyTexture(texture, level, origin, aspect);
    wgpu::ImageCopyBuffer imageCopyBuffer =
        utils::CreateImageCopyBuffer(readback.buffer, readback.offset, bytesPerRow, rowsPerImage);

    wgpu::CommandEncoder encoder = targetDevice.CreateCommandEncoder();
    encoder.CopyTextureToBuffer(&imageCopyTexture, &imageCopyBuffer, &extent);

    wgpu::CommandBuffer commands = encoder.Finish();
    targetDevice.GetQueue().Submit(1, &commands);

    DeferredExpectation deferred;
    deferred.file = file;
    deferred.line = line;
    deferred.readbackSlot = readback.slot;
    deferred.readbackOffset = readback.offset;
    deferred.size = size;
    deferred.rowBytes = extent.width * dataSize;
    deferred.bytesPerRow = bytesPerRow;
    deferred.expectation.reset(expectation);

    // This expectation might be called from multiple threads
    Mutex::AutoLock lg(&mMutex);

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
    shaderSource << "const width : u32 = " << width << "u;\n";
    shaderSource << "@group(0) @binding(0) var tex : " << wgslTextureType << ";\n";
    shaderSource << R"(
        struct Result {
            values : array<f32>
        }
        @group(0) @binding(1) var<storage, read_write> result : Result;
    )";
    shaderSource << "const componentCount : u32 = " << componentCount << "u;\n";
    shaderSource << "const sampleCount : u32 = " << sampleCount << "u;\n";

    shaderSource << "fn doTextureLoad(t: " << wgslTextureType
                 << ", coord: vec2i, sample: u32, component: u32) -> f32";
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
        @compute @workgroup_size(1) fn main(
            @builtin(global_invocation_id) GlobalInvocationId : vec3u
        ) {
            let baseOutIndex = GlobalInvocationId.y * width + GlobalInvocationId.x;
            for (var s = 0u; s < sampleCount; s = s + 1u) {
                for (var c = 0u; c < componentCount; c = c + 1u) {
                    result.values[
                        baseOutIndex * sampleCount * componentCount +
                        s * componentCount +
                        c
                    ] = doTextureLoad(tex, vec2i(GlobalInvocationId.xy), s, c);
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
        @vertex
        fn main(@builtin(vertex_index) VertexIndex : u32) -> @builtin(position) vec4f {
            var pos = array(
                vec2f(-1.0, -1.0),
                vec2f( 3.0, -1.0),
                vec2f(-1.0,  3.0));
            return vec4f(pos[VertexIndex], 0.0, 1.0);
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

            @fragment
            fn main(@builtin(position) FragCoord : vec4f) -> FragmentOut {
                var output : FragmentOut;
                output.result = 1u;
                output.fragDepth = textureLoad(texture0, vec2i(FragCoord.xy), 0)[0];
                return output;
            })");
    } else {
        pipelineDescriptor.cFragment.module = utils::CreateShaderModule(device, R"(
            @fragment
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

void DawnTestBase::WaitABit(wgpu::Instance targetInstance) {
    if (targetInstance == nullptr) {
        targetInstance = mInstance;
    }
    if (targetInstance != nullptr) {
        targetInstance.ProcessEvents();
    }
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
    // Callback might be invoked on another thread that calls the same WaitABit() method, not
    // necessarily the current thread. So we need to use atomic here.
    std::atomic<bool> done(false);
    device.GetQueue().OnSubmittedWorkDone(
        0u,
        [](WGPUQueueWorkDoneStatus, void* userdata) {
            *static_cast<std::atomic<bool>*>(userdata) = true;
        },
        &done);
    while (!done.load()) {
        WaitABit();
    }
}

DawnTestBase::ReadbackReservation DawnTestBase::ReserveReadback(wgpu::Device targetDevice,
                                                                uint64_t readbackSize) {
    ReadbackSlot slot;
    slot.device = targetDevice;
    slot.bufferSize = readbackSize;

    // Create and initialize the slot buffer so that it won't unexpectedly affect the count of
    // resource lazy clear in the tests.
    const std::vector<uint8_t> initialBufferData(readbackSize, 0u);
    slot.buffer =
        utils::CreateBufferFromData(targetDevice, initialBufferData.data(), readbackSize,
                                    wgpu::BufferUsage::MapRead | wgpu::BufferUsage::CopyDst);

    // This readback might be called from multiple threads
    Mutex::AutoLock lg(&mMutex);

    ReadbackReservation reservation;
    reservation.device = targetDevice;
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
    while (mNumPendingMapOperations.load(std::memory_order_acquire) != 0) {
        WaitABit();
    }
}

// static
void DawnTestBase::SlotMapCallback(WGPUBufferMapAsyncStatus status, void* userdata_) {
    DAWN_ASSERT(status == WGPUBufferMapAsyncStatus_Success ||
                status == WGPUBufferMapAsyncStatus_DeviceLost);
    std::unique_ptr<MapReadUserdata> userdata(static_cast<MapReadUserdata*>(userdata_));
    DawnTestBase* test = userdata->test;

    Mutex::AutoLock lg(&test->mMutex);

    ReadbackSlot* slot = &test->mReadbackSlots[userdata->slot];
    if (status == WGPUBufferMapAsyncStatus_Success) {
        slot->mappedData = slot->buffer.GetConstMappedRange();
        ASSERT(slot->mappedData != nullptr);
    } else {
        slot->mappedData = nullptr;
    }

    test->mNumPendingMapOperations.fetch_sub(1, std::memory_order_release);
}

void DawnTestBase::ResolveExpectations() {
    for (const auto& expectation : mDeferredExpectations) {
        EXPECT_TRUE(mReadbackSlots[expectation.readbackSlot].mappedData != nullptr);

        // Get a pointer to the mapped copy of the data for the expectation.
        const char* data =
            static_cast<const char*>(mReadbackSlots[expectation.readbackSlot].mappedData);

        // Handle the case where the device was lost so the expected data couldn't be read back.
        if (data == nullptr) {
            InfoLog() << "Skipping deferred expectation because the device was lost";
            continue;
        }

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

std::unique_ptr<platform::Platform> DawnTestBase::CreateTestPlatform() {
    return nullptr;
}

void DawnTestBase::ResolveDeferredExpectationsNow() {
    FlushWire();

    MapSlotsSynchronously();

    Mutex::AutoLock lg(&mMutex);
    ResolveExpectations();

    mDeferredExpectations.clear();
    for (size_t i = 0; i < mReadbackSlots.size(); ++i) {
        mReadbackSlots[i].buffer.Unmap();
    }
}

bool utils::RGBA8::operator==(const utils::RGBA8& other) const {
    return r == other.r && g == other.g && b == other.b && a == other.a;
}

bool utils::RGBA8::operator!=(const utils::RGBA8& other) const {
    return !(*this == other);
}

bool utils::RGBA8::operator<=(const utils::RGBA8& other) const {
    return (r <= other.r && g <= other.g && b <= other.b && a <= other.a);
}

bool utils::RGBA8::operator>=(const utils::RGBA8& other) const {
    return (r >= other.r && g >= other.g && b >= other.b && a >= other.a);
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
testing::AssertionResult CheckImpl<utils::RGBA8>(const utils::RGBA8& expected,
                                                 const utils::RGBA8& actual,
                                                 const utils::RGBA8& tolerance) {
    if (abs(expected.r - actual.r) > tolerance.r || abs(expected.g - actual.g) > tolerance.g ||
        abs(expected.b - actual.b) > tolerance.b || abs(expected.a - actual.a) > tolerance.a) {
        return tolerance == utils::RGBA8{}
                   ? testing::AssertionFailure() << expected << ", actual " << actual
                   : testing::AssertionFailure()
                         << "within " << tolerance << " of " << expected << ", actual " << actual;
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

template <>
testing::AssertionResult CheckImpl<uint16_t>(const uint16_t& expected,
                                             const uint16_t& actual,
                                             const uint16_t& tolerance) {
    if (abs(static_cast<int32_t>(expected) - static_cast<int32_t>(actual)) > tolerance) {
        return tolerance == 0 ? testing::AssertionFailure() << expected << ", actual " << actual
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

template <typename T>
ExpectConstant<T>::ExpectConstant(T constant) : mConstant(constant) {}

template <typename T>
uint32_t ExpectConstant<T>::DataSize() {
    return sizeof(T);
}

template <typename T>
testing::AssertionResult ExpectConstant<T>::Check(const void* data, size_t size) {
    DAWN_ASSERT(size % DataSize() == 0 && size > 0);
    const T* actual = static_cast<const T*>(data);

    for (size_t i = 0; i < size / DataSize(); ++i) {
        if (actual[i] != mConstant) {
            return testing::AssertionFailure()
                   << "Expected data[" << i << "] to match constant value " << mConstant
                   << ", actual " << actual[i] << std::endl;
        }
    }

    return testing::AssertionSuccess();
}

template class ExpectConstant<float>;

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
template class ExpectEq<utils::RGBA8>;
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

template class ExpectBetweenColors<utils::RGBA8>;
}  // namespace detail
}  // namespace dawn

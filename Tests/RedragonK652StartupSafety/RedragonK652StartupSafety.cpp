#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include <bcrypt.h>

#include <algorithm>
#include <array>
#include <atomic>
#include <cstdlib>
#include <initializer_list>
#include <iostream>
#include <limits>
#include <string>
#include <thread>
#include <vector>

#include "RedragonK652Controller.h"
#include "RedragonK652DeviceRegistry.h"
#include "RGBController_RedragonK652.h"
#include "RGBControllerKeyNames.h"

namespace
{
    static constexpr std::size_t GREEN_PLANE_OFFSET = 0x7E;
    static constexpr std::size_t BLUE_PLANE_OFFSET  = 0xFC;

    static constexpr const char* STATIC_RED_B8_SHA256 =
        "A4B189A05504EEE7942F831F704C2DBF8D618291B5CE70F6F5F78E291A783FAF";
    static constexpr const char* STATIC_B6_SHA256 =
        "674EF7A8E0F5D7B59CEE9B386705FFD5A04D5A347F43810A7B582B6A0E31781F";
    static constexpr const char* DIRECT_ESC_RED_BC_SHA256 =
        "609A1FC0C26AB64B72DA386DDA7D39A4AAE0619F73C72B91B712CB13EFAD8A72";
    static constexpr const char* DIRECT_C0_SHA256 =
        "3EE8E62BCBBD84F77AD1429F94EE5E6654404F440C07D82A60E12F3700853395";
    static constexpr const char* DIRECT_B6_SHA256 =
        "F0A4F6BB0AA4EAFF336AA2BC8F3004AE581027F1E00601BDA5A5CE351CCF20A3";

    static constexpr std::array<unsigned char, 8> RESERVED_INDICES =
    {
        0x53, 0x67, 0x74, 0x75, 0x77, 0x78, 0x7A, 0x7C
    };

    struct ExpectedLED
    {
        const char*   name;
        unsigned char firmware_index;
    };

    const std::array<ExpectedLED, redragon_k652::LED_COUNT> EXPECTED_LEDS =
    {{
        { KEY_EN_ESCAPE,          0x08 }, { KEY_EN_F1,              0x09 },
        { KEY_EN_F2,              0x0A }, { KEY_EN_F3,              0x0B },
        { KEY_EN_F4,              0x0C }, { KEY_EN_F5,              0x0D },
        { KEY_EN_F6,              0x0E }, { KEY_EN_F7,              0x0F },
        { KEY_EN_F8,              0x10 }, { KEY_EN_F9,              0x11 },
        { KEY_EN_F10,             0x12 }, { KEY_EN_F11,             0x13 },
        { KEY_EN_F12,             0x14 }, { KEY_EN_PRINT_SCREEN,    0x15 },
        { KEY_EN_PAUSE_BREAK,     0x16 }, { KEY_EN_DELETE,          0x17 },

        { KEY_EN_BACK_TICK,       0x1D }, { KEY_EN_1,               0x1E },
        { KEY_EN_2,               0x1F }, { KEY_EN_3,               0x20 },
        { KEY_EN_4,               0x21 }, { KEY_EN_5,               0x22 },
        { KEY_EN_6,               0x23 }, { KEY_EN_7,               0x24 },
        { KEY_EN_8,               0x25 }, { KEY_EN_9,               0x26 },
        { KEY_EN_0,               0x27 }, { KEY_EN_MINUS,           0x28 },
        { KEY_EN_EQUALS,          0x29 }, { KEY_EN_BACKSPACE,       0x2A },
        { KEY_EN_HOME,            0x2B },

        { KEY_EN_TAB,             0x32 }, { KEY_EN_Q,               0x33 },
        { KEY_EN_W,               0x34 }, { KEY_EN_E,               0x35 },
        { KEY_EN_R,               0x36 }, { KEY_EN_T,               0x37 },
        { KEY_EN_Y,               0x38 }, { KEY_EN_U,               0x39 },
        { KEY_EN_I,               0x3A }, { KEY_EN_O,               0x3B },
        { KEY_EN_P,               0x3C }, { KEY_EN_LEFT_BRACKET,    0x3D },
        { KEY_EN_RIGHT_BRACKET,   0x3E }, { KEY_EN_ANSI_BACK_SLASH, 0x3F },
        { KEY_EN_END,             0x40 },

        { KEY_EN_CAPS_LOCK,       0x47 }, { KEY_EN_A,               0x48 },
        { KEY_EN_S,               0x49 }, { KEY_EN_D,               0x4A },
        { KEY_EN_F,               0x4B }, { KEY_EN_G,               0x4C },
        { KEY_EN_H,               0x4D }, { KEY_EN_J,               0x4E },
        { KEY_EN_K,               0x4F }, { KEY_EN_L,               0x50 },
        { KEY_EN_SEMICOLON,       0x51 }, { KEY_EN_QUOTE,           0x52 },
        { KEY_EN_ANSI_ENTER,      0x54 }, { KEY_EN_PAGE_UP,         0x55 },

        { KEY_EN_LEFT_SHIFT,      0x5C }, { KEY_EN_Z,               0x5D },
        { KEY_EN_X,               0x5E }, { KEY_EN_C,               0x5F },
        { KEY_EN_V,               0x60 }, { KEY_EN_B,               0x61 },
        { KEY_EN_N,               0x62 }, { KEY_EN_M,               0x63 },
        { KEY_EN_COMMA,           0x64 }, { KEY_EN_PERIOD,          0x65 },
        { KEY_EN_FORWARD_SLASH,   0x66 }, { KEY_EN_RIGHT_SHIFT,     0x68 },
        { KEY_EN_UP_ARROW,        0x69 }, { KEY_EN_PAGE_DOWN,       0x6A },

        { KEY_EN_LEFT_CONTROL,    0x71 }, { KEY_EN_LEFT_WINDOWS,    0x72 },
        { KEY_EN_LEFT_ALT,        0x73 }, { KEY_EN_SPACE,           0x76 },
        { KEY_EN_RIGHT_ALT,       0x79 }, { KEY_EN_RIGHT_CONTROL,   0x7B },
        { KEY_EN_LEFT_ARROW,      0x7D }, { KEY_EN_DOWN_ARROW,      0x7E },
        { KEY_EN_RIGHT_ARROW,     0x7F }
    }};

    static constexpr unsigned int MATRIX_NA = 0xFFFFFFFF;
    static constexpr std::array<unsigned int, 96> EXPECTED_MATRIX =
    {{
         0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15,
        16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, MATRIX_NA, 30,
        31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, MATRIX_NA, 45,
        46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, MATRIX_NA, MATRIX_NA, 59,
        60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, MATRIX_NA, MATRIX_NA, 72, 73,
        74, 75, 76, MATRIX_NA, MATRIX_NA, MATRIX_NA, 77, MATRIX_NA,
        MATRIX_NA, MATRIX_NA, 78, MATRIX_NA, 79, 80, 81, 82
    }};

    enum class OperationType
    {
        SetFeature,
        GetFeature
    };

    struct Operation
    {
        OperationType type;
        hid_device* handle;
        std::vector<unsigned char> report;
    };

    std::vector<Operation> operations;
    std::vector<hid_device*> closed_handles;
    std::vector<unsigned int> sleep_durations;
    std::atomic<unsigned int> link_stub_hid_calls{ 0 };

    int FakeSendFeatureReport(hid_device* handle, const unsigned char* data, std::size_t size)
    {
        operations.push_back({ OperationType::SetFeature, handle, std::vector<unsigned char>(data, data + size) });
        return(static_cast<int>(size));
    }

    int FakeGetFeatureReport(hid_device* handle, unsigned char* data, std::size_t size)
    {
        operations.push_back({ OperationType::GetFeature, handle, std::vector<unsigned char>(data, data + size) });

        if(size < 0x8E)
        {
            return(-1);
        }

        std::fill(data, data + size, 0);
        data[0x00] = 0x06;
        data[0x01] = 0x83;
        data[0x02] = 0xB6;
        data[0x0E] = 0x5A;
        data[0x0F] = 0xA5;
        data[0x10] = 0x03;
        data[0x11] = 0x03;
        data[0x12] = 0x03;
        data[0x1C] = 0x55;
        data[0x1D] = 0x55;
        data[0x89] = 0x5A;
        data[0x8A] = 0xA5;
        data[0x8B] = 0x03;
        data[0x8C] = 0x03;
        data[0x8D] = 0x03;
        return(0x8E);
    }

    int FailSendFeatureReport(hid_device* handle, const unsigned char* data, std::size_t size)
    {
        operations.push_back({ OperationType::SetFeature, handle, std::vector<unsigned char>(data, data + size) });
        return(-1);
    }

    int ShortGetFeatureReport(hid_device* handle, unsigned char* data, std::size_t size)
    {
        FakeGetFeatureReport(handle, data, size);
        return(0x8D);
    }

    int BadSignatureGetFeatureReport(hid_device* handle, unsigned char* data, std::size_t size)
    {
        const int result = FakeGetFeatureReport(handle, data, size);

        if(size > 0x1C)
        {
            data[0x1C] = 0x54;
        }

        return(result);
    }

    void FakeClose(hid_device* handle)
    {
        closed_handles.push_back(handle);
    }

    void FakeSleep(unsigned int milliseconds)
    {
        sleep_durations.push_back(milliseconds);
    }

    const RedragonK652HIDOperations fake_hid_operations =
    {
        FakeSendFeatureReport,
        FakeGetFeatureReport,
        FakeClose,
        FakeSleep
    };

    const RedragonK652HIDOperations failed_send_operations =
    {
        FailSendFeatureReport,
        FakeGetFeatureReport,
        FakeClose,
        FakeSleep
    };

    const RedragonK652HIDOperations short_read_operations =
    {
        FakeSendFeatureReport,
        ShortGetFeatureReport,
        FakeClose,
        FakeSleep
    };

    const RedragonK652HIDOperations bad_signature_operations =
    {
        FakeSendFeatureReport,
        BadSignatureGetFeatureReport,
        FakeClose,
        FakeSleep
    };

    [[noreturn]] void Fail(const std::string& message)
    {
        std::cerr << "FAIL: " << message << std::endl;
        std::exit(1);
    }

    void Require(bool condition, const std::string& message)
    {
        if(!condition)
        {
            Fail(message);
        }
    }

    void RequireSleeps(std::initializer_list<unsigned int> expected, const std::string& context)
    {
        Require(sleep_durations == std::vector<unsigned int>(expected),
                context + " must preserve the exact captured pacing sequence");
        sleep_durations.clear();
    }

    struct BCryptAlgorithm
    {
        BCRYPT_ALG_HANDLE handle = nullptr;

        ~BCryptAlgorithm()
        {
            if(handle)
            {
                BCryptCloseAlgorithmProvider(handle, 0);
            }
        }
    };

    struct BCryptHash
    {
        BCRYPT_HASH_HANDLE handle = nullptr;

        ~BCryptHash()
        {
            if(handle)
            {
                BCryptDestroyHash(handle);
            }
        }
    };

    std::string Sha256(const std::vector<unsigned char>& bytes)
    {
        Require(bytes.size() <= std::numeric_limits<ULONG>::max(), "SHA-256 input is too large for BCrypt");

        BCryptAlgorithm algorithm;
        Require(BCRYPT_SUCCESS(BCryptOpenAlgorithmProvider(&algorithm.handle, BCRYPT_SHA256_ALGORITHM, nullptr, 0)),
                "BCryptOpenAlgorithmProvider(SHA-256) failed");

        DWORD object_size = 0;
        DWORD result_size = 0;
        Require(BCRYPT_SUCCESS(BCryptGetProperty(algorithm.handle, BCRYPT_OBJECT_LENGTH,
                                                 reinterpret_cast<PUCHAR>(&object_size), sizeof(object_size), &result_size, 0))
             && result_size == sizeof(object_size),
                "BCrypt SHA-256 object-size query failed");

        DWORD digest_size = 0;
        Require(BCRYPT_SUCCESS(BCryptGetProperty(algorithm.handle, BCRYPT_HASH_LENGTH,
                                                 reinterpret_cast<PUCHAR>(&digest_size), sizeof(digest_size), &result_size, 0))
             && result_size == sizeof(digest_size)
             && digest_size == 32,
                "BCrypt SHA-256 digest-size query failed");

        std::vector<unsigned char> object(object_size);
        std::vector<unsigned char> digest(digest_size);
        BCryptHash hash;
        Require(BCRYPT_SUCCESS(BCryptCreateHash(algorithm.handle, &hash.handle, object.data(), object_size,
                                                nullptr, 0, 0)),
                "BCryptCreateHash(SHA-256) failed");
        Require(BCRYPT_SUCCESS(BCryptHashData(hash.handle, const_cast<PUCHAR>(bytes.data()),
                                              static_cast<ULONG>(bytes.size()), 0)),
                "BCryptHashData(SHA-256) failed");
        Require(BCRYPT_SUCCESS(BCryptFinishHash(hash.handle, digest.data(), digest_size, 0)),
                "BCryptFinishHash(SHA-256) failed");

        static constexpr char HEX[] = "0123456789ABCDEF";
        std::string result;
        result.reserve(digest.size() * 2);

        for(unsigned char value : digest)
        {
            result.push_back(HEX[value >> 4]);
            result.push_back(HEX[value & 0x0F]);
        }

        return(result);
    }

    std::vector<unsigned char> ExpectedReadRequest()
    {
        std::vector<unsigned char> report(redragon_k652::REPORT_SIZE, 0);
        report[0] = 0x06;
        return(report);
    }

    void RequireQueryRead(std::size_t start, hid_device* command_handle, hid_device* data_handle,
                          const std::string& context)
    {
        Require(operations.size() >= start + 2, context + " must contain a query and read");
        Require(operations[start].type == OperationType::SetFeature
             && operations[start].handle == command_handle
             && operations[start].report == std::vector<unsigned char>({ 0x05, 0x83, 0xB6, 0x00, 0x00, 0x00 }),
                context + " query must be the exact captured six-byte report on the command handle");
        Require(operations[start + 1].type == OperationType::GetFeature
             && operations[start + 1].handle == data_handle
             && operations[start + 1].report == ExpectedReadRequest(),
                context + " read must request a zero-initialized 1032-byte report 0x06 on the data handle");
    }

    void TestValidationFailures(hid_device* command_handle, hid_device* data_handle)
    {
        operations.clear();
        sleep_durations.clear();

        Require(!RedragonK652Controller::ValidateConfiguration(command_handle, data_handle, &failed_send_operations),
                "short/failed configuration query write must reject detection");
        Require(operations.size() == 1
             && operations[0].type == OperationType::SetFeature
             && operations[0].handle == command_handle
             && operations[0].report == std::vector<unsigned char>({ 0x05, 0x83, 0xB6, 0x00, 0x00, 0x00 }),
                "failed validation write must attempt only the exact captured query");
        Require(sleep_durations.empty(), "failed query write must stop before pacing or reading");

        operations.clear();
        Require(!RedragonK652Controller::ValidateConfiguration(command_handle, data_handle, &short_read_operations),
                "configuration response shorter than the captured signature must reject detection");
        Require(operations.size() == 2, "short configuration response must stop after one query/read cycle");
        RequireQueryRead(0, command_handle, data_handle, "short-response validation");
        RequireSleeps({ 20, 20 }, "short-response validation");

        operations.clear();
        Require(!RedragonK652Controller::ValidateConfiguration(command_handle, data_handle, &bad_signature_operations),
                "configuration response with a mismatched captured marker must reject detection");
        Require(operations.size() == 2, "bad configuration signature must stop after one query/read cycle");
        RequireQueryRead(0, command_handle, data_handle, "bad-signature validation");
        RequireSleeps({ 20, 20 }, "bad-signature validation");

        operations.clear();
    }

    const Operation& RequireDataReport(std::size_t index, hid_device* data_handle,
                                       unsigned char command, unsigned char subtype,
                                       const std::string& context)
    {
        Require(index < operations.size(), context + " report is missing");
        const Operation& operation = operations[index];
        Require(operation.type == OperationType::SetFeature
             && operation.handle == data_handle
             && operation.report.size() == redragon_k652::REPORT_SIZE
             && operation.report[0x00] == 0x06
             && operation.report[0x01] == command
             && operation.report[0x02] == subtype,
                context + " must be the expected 1032-byte feature report on the data handle");
        return(operation);
    }

    void RequireReservedPlanesZero(const std::vector<unsigned char>& report, const std::string& context)
    {
        Require(report.size() == redragon_k652::REPORT_SIZE, context + " report has the wrong size");

        for(unsigned char index : RESERVED_INDICES)
        {
            Require(report[index] == 0
                 && report[index + GREEN_PLANE_OFFSET] == 0
                 && report[index + BLUE_PLANE_OFFSET] == 0,
                    context + " must leave every reserved/Fn RGB plane byte zero");
        }

        Require(report[0x7A] == 0 && report[0xF8] == 0 && report[0x176] == 0,
                context + " must leave all three candidate Fn bytes zero");
    }

    void RequireStaticTransaction(std::size_t start, hid_device* command_handle, hid_device* data_handle,
                                  RGBColor expected_color, const std::vector<unsigned char>* expected_b8,
                                  const std::string& context)
    {
        Require(operations.size() == start + 4, context + " must add exactly query/read/B8/B6");
        RequireQueryRead(start, command_handle, data_handle, context);
        const Operation& b8 = RequireDataReport(start + 2, data_handle, 0x08, 0xB8, context + " B8");
        const Operation& b6 = RequireDataReport(start + 3, data_handle, 0x03, 0xB6, context + " B6");

        Require(b8.report[0x1D] == RGBGetRValue(expected_color)
             && b8.report[0x1E] == RGBGetGValue(expected_color)
             && b8.report[0x1F] == RGBGetBValue(expected_color),
                context + " must place arbitrary 8-bit color values in the captured R/G/B fields");
        Require(b6.report[0x15] == redragon_k652::MODE_STATIC,
                context + " B6 must select captured static mode 0x01");
        Require(Sha256(b6.report) == STATIC_B6_SHA256,
                context + " B6 must match the complete captured static template");

        if(expected_b8)
        {
            Require(b8.report == *expected_b8, context + " B8 may differ from the captured template only at R/G/B");
        }

        RequireSleeps({ 20, 20, 50, 50, 20 }, context);
    }

    const std::vector<unsigned char>& RequireDirectTransaction(std::size_t start, hid_device* command_handle,
                                                                hid_device* data_handle, const std::string& context)
    {
        Require(operations.size() == start + 5, context + " must add exactly query/read/BC/C0/B6");
        RequireQueryRead(start, command_handle, data_handle, context);
        const Operation& bc = RequireDataReport(start + 2, data_handle, 0x09, 0xBC, context + " BC");
        const Operation& c0 = RequireDataReport(start + 3, data_handle, 0x09, 0xC0, context + " C0");
        const Operation& b6 = RequireDataReport(start + 4, data_handle, 0x03, 0xB6, context + " B6");

        RequireReservedPlanesZero(bc.report, context + " BC");
        Require(Sha256(c0.report) == DIRECT_C0_SHA256,
                context + " C0 must match the complete captured invariant template");
        Require(b6.report[0x15] == redragon_k652::MODE_DIRECT,
                context + " B6 must select captured direct mode 0x15");
        Require(Sha256(b6.report) == DIRECT_B6_SHA256,
                context + " B6 must match the complete captured direct template");
        RequireSleeps({ 20, 20, 50, 50, 50, 20 }, context);
        return(bc.report);
    }

    std::size_t FindLED(const RGBController_RedragonK652& controller, unsigned int firmware_index)
    {
        for(std::size_t led_index = 0; led_index < controller.leds.size(); led_index++)
        {
            if(controller.leds[led_index].value == firmware_index)
            {
                return(led_index);
            }
        }

        Fail("firmware index is not exposed by the RGB controller");
    }

    void RequireLEDMap(const RGBController_RedragonK652& controller)
    {
        Require(controller.leds.size() == redragon_k652::LED_COUNT,
                "RGB controller must expose exactly 83 independently configurable LEDs");
        Require(controller.colors.size() == redragon_k652::LED_COUNT,
                "RGB controller color vector must contain exactly 83 entries");
        Require((controller.flags & CONTROLLER_FLAG_NO_PROFILE_APPLY) != 0,
                "experimental K652 must reject automatic and manual profile application");
        Require(controller.zones.size() == 1
             && controller.zones[0].matrix_map.height == 6
             && controller.zones[0].matrix_map.width == 16
             && controller.zones[0].matrix_map.map == std::vector<unsigned int>(EXPECTED_MATRIX.begin(), EXPECTED_MATRIX.end()),
                "K652 must retain the independently specified 6x16 matrix including the Fn hole");

        std::array<bool, 128> seen{};
        for(std::size_t led_index = 0; led_index < controller.leds.size(); led_index++)
        {
            const unsigned int firmware_index = controller.leds[led_index].value;
            Require(firmware_index == EXPECTED_LEDS[led_index].firmware_index
                 && redragon_k652::KEY_INDICES[led_index] == EXPECTED_LEDS[led_index].firmware_index,
                    "RGB LED order and firmware table must match the independent K652 golden map");
            Require(controller.leds[led_index].name == EXPECTED_LEDS[led_index].name,
                    "RGB LED names must match the independent K652 golden map");
            Require(firmware_index < seen.size() && !seen[firmware_index],
                    "every exposed RGB LED must have one unique in-range firmware index");
            seen[firmware_index] = true;

            Require(std::find(RESERVED_INDICES.begin(), RESERVED_INDICES.end(), firmware_index) == RESERVED_INDICES.end(),
                    "reserved and candidate Fn indices must not be exposed");
        }
    }

    RedragonK652DeviceClaim TestRegistryClaims(const std::array<unsigned char, 16>& first_container)
    {
        const std::array<unsigned char, 16> second_container =
            { 0x52, 0x65, 0x64, 0x72, 0x61, 0x67, 0x6F, 0x6E, 0x4B, 0x36, 0x35, 0x32, 0x00, 0x00, 0x00, 0x02 };
        const std::array<unsigned char, 16> concurrent_container =
            { 0x52, 0x65, 0x64, 0x72, 0x61, 0x67, 0x6F, 0x6E, 0x4B, 0x36, 0x35, 0x32, 0x00, 0x00, 0x00, 0x03 };

        Require(!RedragonK652DeviceRegistry::Claim(nullptr, first_container.size()),
                "missing container metadata must fail closed");
        Require(!RedragonK652DeviceRegistry::Claim(first_container.data(), 0),
                "zero-length container metadata must fail closed");

        RedragonK652DeviceClaim first_claim =
            RedragonK652DeviceRegistry::Claim(first_container.data(), first_container.size());
        RedragonK652DeviceClaim second_claim =
            RedragonK652DeviceRegistry::Claim(second_container.data(), second_container.size());
        Require(first_claim && second_claim, "two distinct Windows container claims must coexist");
        Require(!RedragonK652DeviceRegistry::Claim(first_container.data(), first_container.size())
             && !RedragonK652DeviceRegistry::Claim(second_container.data(), second_container.size()),
                "duplicate container claims must be rejected while their owners are alive");

        second_claim.reset();
        second_claim = RedragonK652DeviceRegistry::Claim(second_container.data(), second_container.size());
        Require(static_cast<bool>(second_claim), "released distinct-container claim must be reclaimable");
        second_claim.reset();

        static constexpr std::size_t THREAD_COUNT = 8;
        std::array<RedragonK652DeviceClaim, THREAD_COUNT> concurrent_claims;
        std::array<std::thread, THREAD_COUNT> threads;
        std::atomic<unsigned int> ready{ 0 };
        std::atomic<bool> start{ false };

        for(std::size_t thread_index = 0; thread_index < THREAD_COUNT; thread_index++)
        {
            threads[thread_index] = std::thread([&, thread_index]()
            {
                ready.fetch_add(1);
                while(!start.load())
                {
                    std::this_thread::yield();
                }

                concurrent_claims[thread_index] = RedragonK652DeviceRegistry::Claim(
                    concurrent_container.data(), concurrent_container.size());
            });
        }

        while(ready.load() != THREAD_COUNT)
        {
            std::this_thread::yield();
        }
        start.store(true);

        for(std::thread& thread : threads)
        {
            thread.join();
        }

        const std::size_t winners = static_cast<std::size_t>(std::count_if(
            concurrent_claims.begin(), concurrent_claims.end(),
            [](const RedragonK652DeviceClaim& claim) { return(static_cast<bool>(claim)); }));
        Require(winners == 1, "concurrent claims for one container must have exactly one winner");

        RedragonK652DeviceClaim winning_claim;
        for(const RedragonK652DeviceClaim& claim : concurrent_claims)
        {
            if(claim)
            {
                winning_claim = claim;
                break;
            }
        }

        for(RedragonK652DeviceClaim& claim : concurrent_claims)
        {
            claim.reset();
        }
        Require(!RedragonK652DeviceRegistry::Claim(concurrent_container.data(), concurrent_container.size()),
                "a copied winning claim must retain concurrent ownership");
        winning_claim.reset();

        RedragonK652DeviceClaim reclaimed =
            RedragonK652DeviceRegistry::Claim(concurrent_container.data(), concurrent_container.size());
        Require(static_cast<bool>(reclaimed), "released concurrent container claim must be reclaimable");
        reclaimed.reset();

        return(first_claim);
    }
}

/*
 * Link-time hidapi stubs make this executable structurally incapable of
 * reaching a physical HID device even if a test accidentally uses the
 * controller's default operation table instead of the injected fake table.
 */
extern "C" int HID_API_EXPORT_CALL hid_send_feature_report(hid_device*, const unsigned char*, std::size_t)
{
    link_stub_hid_calls.fetch_add(1);
    return(-1);
}

extern "C" int HID_API_EXPORT_CALL hid_get_feature_report(hid_device*, unsigned char*, std::size_t)
{
    link_stub_hid_calls.fetch_add(1);
    return(-1);
}

extern "C" void HID_API_EXPORT_CALL hid_close(hid_device*)
{
    link_stub_hid_calls.fetch_add(1);
}

int main()
{
    const std::array<unsigned char, 16> container_id =
        { 0x52, 0x65, 0x64, 0x72, 0x61, 0x67, 0x6F, 0x6E, 0x4B, 0x36, 0x35, 0x32, 0x00, 0x00, 0x00, 0x01 };
    RedragonK652DeviceClaim first_claim = TestRegistryClaims(container_id);
    Require(static_cast<bool>(first_claim), "first Windows container must remain claimed for controller construction");
    Require(!RedragonK652DeviceRegistry::Claim(container_id.data(), container_id.size()),
            "duplicate detector callback must be rejected by the container claim before active validation");

    hid_device* command_handle = reinterpret_cast<hid_device*>(0x1000);
    hid_device* data_handle    = reinterpret_cast<hid_device*>(0x2000);

    TestValidationFailures(command_handle, data_handle);

    {
        RedragonK652Controller aliased_handles(command_handle, command_handle, "offline", "Redragon K652",
                                               {}, &fake_hid_operations);
    }
    Require(closed_handles.size() == 1 && closed_handles[0] == command_handle,
            "defensive destruction must close an aliased command/data handle exactly once");
    closed_handles.clear();

    /* Simulate successful detector validation: this is the sole startup I/O. */
    const std::size_t validation_start = operations.size();
    Require(RedragonK652Controller::ValidateConfiguration(command_handle, data_handle, &fake_hid_operations),
            "captured configuration response must validate");
    Require(operations.size() == validation_start + 2,
            "successful active validation must produce exactly one configuration query and one feature-report read");
    RequireQueryRead(validation_start, command_handle, data_handle, "startup validation");
    RequireSleeps({ 20, 20 }, "startup validation");

    RedragonK652Controller* low_level_controller = nullptr;
    {
        low_level_controller = new RedragonK652Controller(command_handle, data_handle, "offline", "Redragon K652",
                                                           first_claim, &fake_hid_operations);
        RGBController_RedragonK652 controller(low_level_controller);
        first_claim.reset();

        RequireLEDMap(controller);

        /* Simulate controller-facing registration and widget-sync calls while the device-local lock is held. */
        const std::size_t startup_io_count = operations.size();
        Require(!low_level_controller->ApplyStatic(ToRGBColor(1, 2, 3)),
                "low-level static apply must remain locked before explicit user authorization");
        Require(!low_level_controller->ApplyDirect(std::vector<RGBColor>(redragon_k652::LED_COUNT, ToRGBColor(1, 2, 3))),
                "low-level direct apply must remain locked before explicit user authorization");
        controller.UpdateMode();
        controller.UpdateLEDs();
        controller.UpdateZoneLEDs(0);
        controller.UpdateSingleLED(0);
        controller.SetCustomMode();
        controller.UpdateMode();
        controller.UpdateLEDs();
        controller.SetModeColor(0, 0, ToRGBColor(7, 8, 9));
        controller.SetActiveMode(0);

        Require(operations.size() == startup_io_count,
                "construction and controller-facing startup calls must not query or write lighting reports");
        Require(sleep_durations.empty(), "locked startup calls must not perform protocol pacing");
        Require(!RedragonK652DeviceRegistry::Claim(container_id.data(), container_id.size()),
                "registered controller must retain exclusive ownership of its Windows container");

        const std::size_t authorization_io_count = operations.size();
        controller.EnableUserLightingControl();
        Require(operations.size() == authorization_io_count,
                "explicit authorization itself must not send a HID report");

        /* Static red: complete captured B8 and B6 goldens. */
        controller.SetModeColor(0, 0, ToRGBColor(255, 0, 0));
        std::size_t transaction_start = operations.size();
        controller.SetActiveMode(0);
        RequireStaticTransaction(transaction_start, command_handle, data_handle, ToRGBColor(255, 0, 0), nullptr,
                                 "Static red");
        const std::vector<unsigned char> static_red_b8 = operations[transaction_start + 2].report;
        Require(Sha256(static_red_b8) == STATIC_RED_B8_SHA256,
                "Static red B8 must match the complete captured report");

        /* All other static reports must be byte-identical except for the observed arbitrary RGB fields. */
        const std::array<RGBColor, 3> additional_static_colors =
        {
            ToRGBColor(0, 255, 0),
            ToRGBColor(0, 0, 255),
            ToRGBColor(128, 64, 32)
        };
        const std::array<const char*, 3> additional_static_names =
        {
            "Static green",
            "Static blue",
            "Static 128/64/32"
        };

        for(std::size_t color_index = 0; color_index < additional_static_colors.size(); color_index++)
        {
            const RGBColor color = additional_static_colors[color_index];
            std::vector<unsigned char> expected_b8 = static_red_b8;
            expected_b8[0x1D] = RGBGetRValue(color);
            expected_b8[0x1E] = RGBGetGValue(color);
            expected_b8[0x1F] = RGBGetBValue(color);

            controller.SetModeColor(0, 0, color);
            transaction_start = operations.size();
            controller.SetActiveMode(0);
            RequireStaticTransaction(transaction_start, command_handle, data_handle, color, &expected_b8,
                                     additional_static_names[color_index]);
        }

        const std::size_t esc_led         = FindLED(controller, 0x08);
        const std::size_t space_led       = FindLED(controller, 0x76);
        const std::size_t right_arrow_led = FindLED(controller, 0x7F);

        controller.SetCustomMode();

        /* Direct Esc red: complete captured BC, C0, and B6 goldens. */
        std::fill(controller.colors.begin(), controller.colors.end(), ToRGBColor(0, 0, 0));
        controller.colors[esc_led] = ToRGBColor(255, 0, 0);
        transaction_start = operations.size();
        controller.UpdateLEDs();
        const std::vector<unsigned char> direct_esc_red_bc =
            RequireDirectTransaction(transaction_start, command_handle, data_handle, "Direct Esc red");
        Require(direct_esc_red_bc[0x008] == 0xFF
             && direct_esc_red_bc[0x086] == 0x00
             && direct_esc_red_bc[0x104] == 0x00,
                "Direct Esc red must use the captured R/G/B planes");
        Require(Sha256(direct_esc_red_bc) == DIRECT_ESC_RED_BC_SHA256,
                "Direct Esc red BC must match the complete captured report");

        std::vector<unsigned char> blank_direct_bc = direct_esc_red_bc;
        blank_direct_bc[0x008] = 0;

        /* Direct Esc green and blue must change only the observed channel-plane byte. */
        const std::array<RGBColor, 2> esc_colors =
        {
            ToRGBColor(0, 255, 0),
            ToRGBColor(0, 0, 255)
        };
        const std::array<std::size_t, 2> esc_offsets = { 0x086, 0x104 };
        const std::array<const char*, 2> esc_names = { "Direct Esc green", "Direct Esc blue" };

        for(std::size_t color_index = 0; color_index < esc_colors.size(); color_index++)
        {
            std::fill(controller.colors.begin(), controller.colors.end(), ToRGBColor(0, 0, 0));
            controller.colors[esc_led] = esc_colors[color_index];
            transaction_start = operations.size();
            controller.UpdateLEDs();
            const std::vector<unsigned char> actual_bc =
                RequireDirectTransaction(transaction_start, command_handle, data_handle, esc_names[color_index]);
            std::vector<unsigned char> expected_bc = blank_direct_bc;
            expected_bc[esc_offsets[color_index]] = 0xFF;
            Require(actual_bc == expected_bc,
                    std::string(esc_names[color_index]) + " must change only the captured channel-plane byte");
        }

        /* Verify captured arbitrary per-channel values without claiming a full-report golden. */
        std::fill(controller.colors.begin(), controller.colors.end(), ToRGBColor(0, 0, 0));
        controller.colors[space_led]       = ToRGBColor(128, 64, 32);
        controller.colors[right_arrow_led] = ToRGBColor(17, 34, 51);
        transaction_start = operations.size();
        controller.UpdateLEDs();
        const std::vector<unsigned char> arbitrary_bc =
            RequireDirectTransaction(transaction_start, command_handle, data_handle,
                                     "Direct Space 128/64/32 + Right Arrow 17/34/51");

        std::vector<unsigned char> expected_arbitrary_bc = blank_direct_bc;
        expected_arbitrary_bc[0x076] = 128;
        expected_arbitrary_bc[0x0F4] = 64;
        expected_arbitrary_bc[0x172] = 32;
        expected_arbitrary_bc[0x07F] = 17;
        expected_arbitrary_bc[0x0FD] = 34;
        expected_arbitrary_bc[0x17B] = 51;
        Require(arbitrary_bc == expected_arbitrary_bc,
                "arbitrary Space and Right Arrow values must appear only in their captured R/G/B plane bytes");

        /* A malformed Direct color count must fail before even the safe configuration query. */
        const std::size_t wrong_count_io = operations.size();
        Require(!low_level_controller->ApplyDirect(std::vector<RGBColor>(redragon_k652::LED_COUNT - 1, 0)),
                "82-color Direct update must be rejected");
        Require(!low_level_controller->ApplyDirect(std::vector<RGBColor>(redragon_k652::LED_COUNT + 1, 0)),
                "84-color Direct update must be rejected");
        Require(operations.size() == wrong_count_io,
                "wrong Direct color counts must produce no query, read, or write");
        Require(sleep_durations.empty(), "wrong Direct color counts must not perform protocol pacing");
    }

    Require(closed_handles.size() == 2,
            "controller destruction must close exactly the two injected HID handles");
    Require(std::count(closed_handles.begin(), closed_handles.end(), command_handle) == 1
         && std::count(closed_handles.begin(), closed_handles.end(), data_handle) == 1,
            "controller destruction must close each injected HID handle exactly once");

    RedragonK652DeviceClaim reclaimed =
        RedragonK652DeviceRegistry::Claim(container_id.data(), container_id.size());
    Require(static_cast<bool>(reclaimed), "controller destruction must release its Windows container claim for unplug/rescan");
    reclaimed.reset();

    Require(link_stub_hid_calls.load() == 0,
            "offline safety test must never reach a real/default hidapi call path");

    std::cout << "PASS: offline K652 startup, packet goldens, RGB planes, registry, and lifetime safety" << std::endl;
    return(0);
}

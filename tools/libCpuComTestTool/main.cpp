/*
 * COPYRIGHT (C) 2019 MITSUBISHI ELECTRIC CORPORATION
 * ALL RIGHTS RESERVED
 */

#include "CpuCom.h"
#include "Log.h"
#include "libCpuCom.h"
#include "libMelcoCommon.h"

#include <chrono>
#include <future>
#include <iomanip>
#include <iostream>
#include <thread>

using namespace com::mitsubishielectric::ahu::common;
using namespace com::mitsubishielectric::ahu::cpucom;
using namespace std::chrono_literals;

struct TEST {
    TEST(std::string label)
        : m_label(label)
        , m_result(true)
    {
        std::cout << "[ RUN      ] " << m_label << std::endl;
        m_startTime = std::chrono::steady_clock::now();
    }

    ~TEST()
    {
        auto duration = std::chrono::steady_clock::now() - m_startTime;
        if (m_result) {
            std::cout << "[       OK ] " << m_label << " ("
                      << std::chrono::duration_cast<std::chrono::milliseconds>(duration).count()
                      << "ms)" << std::endl;
        }
        else {
            std::cout << "[   FAILED ] " << m_label << " ("
                      << std::chrono::duration_cast<std::chrono::milliseconds>(duration).count()
                      << "ms)" << std::endl;
        }
    }

    void setResult(bool value) { m_result = value; }

private:
    std::string m_label;
    std::chrono::steady_clock::time_point m_startTime;
    bool m_result;
};

#define START_TEST(a) TEST t(a)
#define TEST_FAILED t.setResult(false)

int main()
{
    InitializeCommonLogMessages();
    InitializeLibCpuComLogMessages();
    std::unique_ptr<v2::ICpuCom> client = v2::ICpuCom::create();

    client->initialize({}, {});
    client->connect();

    CpuCommand requestCommand = {0xfd, 0x01};
    std::vector<uint8_t> requestData = {0x01, 0x02, 0x03, 0x04, 0x05};
    CpuCommand responseCommand = {0xfd, 0x81};

    const size_t kTimesRepeat = 300;

    {
        START_TEST("Send test");

        client->send(requestCommand, requestData);
    }
    {
        START_TEST("Send test with DeliveryStatus");

        std::promise<bool> p;
        auto f = p.get_future();

        client->send(requestCommand, requestData, [&p](bool status) { p.set_value(status); });

        if (f.wait_for(3s) == std::future_status::ready && !f.get()) {
            TEST_FAILED;
        }
    }
    {
        START_TEST("Wait for delivery confirmation");
        {
            int count = 0;
            auto onCommandDelivered = [&count](bool) { ++count; };
            for (size_t i = 0; i < kTimesRepeat; ++i) {
                client->send({0x11, 0x00}, {}, onCommandDelivered);
            }
            auto response = client->request(requestCommand, requestData, responseCommand);
            response->wait();  // wait for all sent commands processed
            if (count != kTimesRepeat) {
                TEST_FAILED;
            }
        }
    }
    {
        START_TEST("Drop response immediately");
        for (size_t i = 0; i < kTimesRepeat; ++i) {
            client->request(requestCommand, requestData, responseCommand);
        }
    }
    {
        START_TEST("Wait for response");
        for (size_t i = 0; i < kTimesRepeat; ++i) {
            auto response = client->request(requestCommand, requestData, responseCommand);
            using namespace std::chrono_literals;
            if (response->wait_for(1s) == std::future_status::ready) {
                auto data = response->data();
                (void)data;
            }
        }
    }
    {
        std::vector<std::future<bool>> responses;
        START_TEST("Wait for response in another thread");
        {
            for (size_t i = 0; i < kTimesRepeat; ++i) {
                auto response = client->request(requestCommand, requestData, responseCommand);
                responses.push_back(
                    std::async(std::launch::async, [r = std::move(response)]() mutable {
                        if (r->wait_for(5s) == std::future_status::ready)
                            return true;
                        else
                            return false;
                    }));
            }

            int responseCounter = 0;
            for (size_t i = 0; i < kTimesRepeat; ++i) {
                responses[i].wait();

                if (responses[i].get())
                    ++responseCounter;
            }

            std::cout << "Total Requests : " << kTimesRepeat << std::endl;
            std::cout << "Total Responses: " << responseCounter << std::endl;

            if (kTimesRepeat != responseCounter)
                TEST_FAILED;
        }
    }
    TerminateLibCpuComLogMessages();
    TerminateCommonLogMessages();
    return 0;
}

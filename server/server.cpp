#include <simpleble/SimpleBLE.h>
#include <simpleble/AdapterSafe.h>
#include <simpleble/PeripheralSafe.h>
#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include <atomic>

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <local-adapter-mac>\n";
        return 1;
    }
    const std::string adapter_mac = argv[1];

    // Service/characteristic we expect the remote device to notify/write.
    const std::string service_uuid  = "12345678-1234-5678-1234-56789abcdef0";
    const std::string char_uuid     = "abcdefab-1234-5678-1234-56789abcdef0";
    const std::string expect_msg    = "WELCOME";

    // Get adapters (Safe API)
    auto adapters_opt = SimpleBLE::Safe::Adapter::get_adapters();
    if (!adapters_opt.has_value()) {
        std::cerr << "[FATAL] Failed to enumerate adapters\n";
        return 1;
    }

    SimpleBLE::Safe::Adapter chosen_adapter = SimpleBLE::Safe::Adapter(std::move(adapters_opt->at(0))); // fallback
    bool found_adapter = false;
    for (auto &a : *adapters_opt) {
        auto a_addr = a.address();
        if (a_addr.has_value() && a_addr.value() == adapter_mac) {
            chosen_adapter = a;
            found_adapter = true;
            break;
        }
    }
    if (!found_adapter) {
        std::cerr << "[FATAL] Adapter " << adapter_mac << " not found\n";
        return 1;
    }

    std::cout << "[INFO] Using adapter " << adapter_mac << "\n";

    // Start scan for peripherals (5s)
    if (!chosen_adapter.scan_for(5000)) {
        std::cerr << "[FATAL] scan_for failed\n";
        return 1;
    }

    auto results_opt = chosen_adapter.scan_get_results();
    if (!results_opt.has_value()) {
        std::cerr << "[FATAL] Failed to get scan results\n";
        return 1;
    }

    // look for a peripheral that exposes the service/characteristic
    SimpleBLE::Safe::Peripheral target_peripheral = SimpleBLE::Safe::Peripheral(SimpleBLE::Peripheral{});
    bool found_peripheral = false;
    for (auto &p : *results_opt) {
        auto p_addr_opt = p.address();
        std::string p_addr = p_addr_opt.value_or(std::string("<unknown>"));
        std::cout << "[SCAN] Found peripheral " << p_addr << "\n";

        // try to connect and inspect services
        if (!p.connect()) {
            std::cout << "[INFO] could not connect to " << p_addr << "\n";
            continue;
        }

        auto services_opt = p.services();
        if (!services_opt.has_value()) {
            p.disconnect();
            continue;
        }

        for (auto &svc : *services_opt) {
            if (svc.uuid() == service_uuid) {
                // find characteristic in this service
                for (auto &ch : svc.characteristics()) {
                    if (ch.uuid() == char_uuid) {
                        target_peripheral = p;
                        found_peripheral = true;
                        break;
                    }
                }
            }
            if (found_peripheral) break;
        }

        if (!found_peripheral) p.disconnect();
        else break;
    }

    if (!found_peripheral) {
        std::cerr << "[FATAL] No peripheral advertising the expected service/char found\n";
        return 1;
    }

    std::cout << "[INFO] Connected to peripheral, subscribing to characteristic notifications...\n";

    std::atomic<bool> got_message{false};
    std::string received;

    bool ok = target_peripheral.notify(service_uuid, char_uuid, [&](SimpleBLE::ByteArray payload) {
        received.assign(payload.begin(), payload.end());
        std::cout << "[RX] Notification: \"" << received << "\"\n";
        if (received == expect_msg) got_message = true;
    });

    if (!ok) {
        std::cerr << "[FATAL] Failed to subscribe to notifications\n";
        target_peripheral.disconnect();
        return 1;
    }

    // wait up to 30s for expected message
    for (int i = 0; i < 30 && !got_message.load(); ++i) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    target_peripheral.unsubscribe(service_uuid, char_uuid);
    target_peripheral.disconnect();

    if (got_message) {
        std::cout << "[OK] Received expected welcome message\n";
        return 0;
    } else {
        std::cout << "[WARN] Timeout waiting for welcome message\n";
        return 2;
    }
}
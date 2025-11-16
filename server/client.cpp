#include <simpleble/SimpleBLE.h>
#include <thread>
#include <iostream>


int main() {
    if (!SimpleBLE::Adapter::bluetooth_enabled())
    {
        std::cout << "[FATAL] Bluetooth not enabled" << std::endl;
        return 1;
    }
    auto adapters = SimpleBLE::Adapter::get_adapters();
    if (adapters.size() == 0)
    {
        std::cout << "[FATAL] No Bluetooth adapters found" << std::endl;
        return 1;
    }

    auto adapter = adapters[0];
    std::cout << "[INFO] MAC Address of current adapter: " << adapter.address() << std::endl;

    // scan for 5 secs
    std::cout << "[INFO] Scanning for server..." << std::endl;
    adapter.scan_for(10000);

    //get devices found
    auto peripherals = adapter.scan_get_results();

    // print devices found
    std::cout << "[INFO] Devices found: " << peripherals.size() << std::endl;
    /*for (auto& peripheral : peripherals)
    {
        std::string address = peripheral.address();
        std::cout << " - " << address << " | " << peripheral.identifier() << std::endl;
    }*/
    for (auto& peripheral : peripherals)
    {
        if (peripheral.address() == "41:42:dc:a1:57:e8") // replace with your server MAC address
        {
            std::cout << "[INFO] Found server peripheral: " << peripheral.identifier() << " [" << peripheral.address() << "]" << std::endl;
            std::cout << "[INFO] Connecting..." << std::endl;
            peripheral.connect();
            std::cout << "[INFO] Connected." << std::endl;
            std::cout << "[INFO] Sending welcomes..." << std::endl;
            auto services = peripheral.services();
            for (auto& service : services)
            {
                std::cout << " Service: " << service.uuid() << std::endl;
                auto characteristics = service.characteristics();
                for (auto& characteristic : characteristics)
                {
                    std::cout << "  Characteristic: " << characteristic.uuid() << std::endl;
                    if (characteristic.can_write_request())
                    {
                        std::string message = "Hello from client!";
                        SimpleBLE::ByteArray data(message.begin(), message.end());
                        // use write_value instead of write_request (write_request does not exist on this build)
                        peripheral.write_request(service.uuid(), characteristic.uuid(), data);
                        std::cout << "   Sent message: " << message << std::endl;
                    } else {
                        std::cout << "[ERROR] Can't send welcomes to this characteristic. Are you sure the server program is running?" << std::endl;
                    }
                }                
            }
        } else {
            std::cout << "[INFO] Skipping peripheral: " << peripheral.identifier() << " [" << peripheral.address() << "]" << std::endl;
            continue;
        }
    }
    
    return 0;
}
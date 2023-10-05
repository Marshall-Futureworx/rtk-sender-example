#include "driver-interface.h"
#include <iostream>

int DriverInterface::callback_entry(GPSCallbackType type, void* data1, int data2, void* user)
{
    auto self = reinterpret_cast<DriverInterface*>(user);
    return self->callback(type, data1, data2);
}

int DriverInterface::callback(GPSCallbackType type, void* data1, int data2)
{
    switch (type) {
        case GPSCallbackType::readDeviceData:
            return serial_comms_.read(static_cast<uint8_t*>(data1), data2);

        case GPSCallbackType::writeDeviceData:
            return serial_comms_.write(static_cast<const uint8_t*>(data1), data2);

        case GPSCallbackType::setBaudrate:
            return (serial_comms_.set_baudrate(data2) ? 0 : 1);

        case GPSCallbackType::gotRTCMMessage:
            send_rtcm_data((uint8_t*)(data1), data2);
            return 0;

        default:
            // Ignore rest.
            return 0;
    }
}

void DriverInterface::send_rtcm_data(uint8_t* data, int data_len)
{
    if (!rtk_plugin_) {
        if (mavsdk_.systems().empty()) {
            printf("No mavlink system available yet\n");
            return;
        }

        rtk_plugin_ = std::make_shared<mavsdk::Rtk>(mavsdk_.systems()[0]);
        telemetry_plugin_ = std::make_shared<mavsdk::Telemetry>(mavsdk_.systems()[0]);
    }

    mavsdk::Rtk::RtcmData rtcm_data;
    rtcm_data.data = std::string((char*)data, static_cast<int>(data_len));
    mavsdk::Rtk::Result result = rtk_plugin_->send_rtcm_data(rtcm_data);
    if (result != mavsdk::Rtk::Result::Success) {
        std::cout << "Error sending RTCM data: " << result << std::endl;
    } else {
        std::cout << "Successfully sent RTCM data" << std::endl;
    }

    std::cout << "Fix type: " << telemetry_plugin_->gps_info().fix_type << '\n';
}

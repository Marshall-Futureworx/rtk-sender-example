#include <cstdio>
#include <memory>

#include "PX4-GPSDrivers/src/gps_helper.h"
#include "PX4-GPSDrivers/src/ubx.h"
#include <QSerialPort>
#include "driver-interface.h"
#include <chrono>
#include <thread>

using namespace std::chrono;

int main(int argc, char* argv[])
{
    printf("Starting rtk-sender-example sees.ai mod version v1.2\n");
    printf("Using PX4-GPSDrivers commit 93168099b4c17d2612a28d83ed15e6542a578920\n");

    if (argc != 4) {
        printf("\n");
        printf("usage: %s <serial device> <baudrate> <mavlink connection>\n", argv[0]);
        printf("e.g.: %s /dev/ttyUSB0 38400 udp://:14550\n", argv[0]);
        printf("Note: use baudrate 0 to determine baudrate automatically\n");
        return 1;
    }


    unsigned baudrate = std::stoi(argv[2]);

    SerialComms serial_comms;
    if (!serial_comms.init(argv[1])) {
        return 2;
    }
    
    QSerialPort serial;
    serial.setPortName(argv[1]);
    
    if (!serial.open(QIODevice::ReadWrite)) {
        int retries = 60;
        // Give the device some time to come up. In some cases the device is not
        // immediately accessible right after startup for some reason. This can take 10-20s.
        while (retries-- > 0 && serial.error() == QSerialPort::PermissionError) {
            std::cout << "Cannot open device... retrying";
            std::this_thread::sleep_for(milliseconds(500));
            if (serial.open(QIODevice::ReadWrite)) {
                serial.clearError();
                break;
            }
        }
        if (serial.error() != QSerialPort::NoError) {
            std::cout << "GPS: Failed to open Serial Device" << argv[1] << ": " << serial.errorString();
            return 1;
        }
    }
    
    serial.setBaudrate(QSerialPort::Baud9600);
    serial.setDataBits(QSerialPort::Data8);
    serial.setParity(QSerialPort::NoParity);
    serial.setStopBits(QSerialPort::OneStop);
    serial.setFlowControl(QSerialPort::NoFlowControl);

    mavsdk::Mavsdk mavsdk;

    // Required when connecting to a flight controller directly via USB.
    mavsdk::Mavsdk::Configuration config{mavsdk::Mavsdk::Configuration::UsageType::GroundStation};
    config.set_always_send_heartbeats(true);
    mavsdk.set_configuration(config);

    auto connection_result = mavsdk.add_any_connection(argv[3]);
    if (connection_result != mavsdk::ConnectionResult::Success) {
        printf("Mavsdk connection failed\n");
        return 3;
    }

    DriverInterface driver_interface(serial, mavsdk);

    auto driver = std::make_unique<GPSDriverUBX>(
            GPSDriverUBX::Interface::UART,
            &DriverInterface::callback_entry, &driver_interface,
            &driver_interface.gps_pos, &driver_interface.sat_info);

    //constexpr auto survey_minimum_m = 5;
    //constexpr auto survey_duration_s = 20;
    //driver->setSurveyInSpecs(survey_minimum_m * 10000, survey_duration_s);
    
    // GPS Lavant hard coded for now to test.
    // Note position accuracy is in mm!
    driver->setBasePosition(50.867740347, -0.791097831, 87.70, 130);


    GPSHelper::GPSConfig gps_config {};
    // to test if RTCM is not available
    //gps_config.output_mode = GPSHelper::OutputMode::GPS;
    gps_config.output_mode = GPSHelper::OutputMode::RTCM;

    if (driver->configure(baudrate, gps_config) != 0) {
        printf("configure failed\n");
        return 4;
    }

    printf("configure done!\n");

    const unsigned timeout_ms = 1200;

    while (true) {
        // Keep running, and don't stop on timeout.
        // Every now and then it timeouts but I'm not sure if that's actually
        // warranted given correct messages are still arriving.
        int ret = 0;
        
        auto time_start = high_resolution_clock::now();
        ret = driver->receive(timeout_ms);
        auto time_execution = duration_cast<milliseconds>(high_resolution_clock::now() - time_start);
        // printf("time lapsed:%d\n", int(time_execution.count()));

        if (ret < 0){           
            // Timedout
            // printf("timed out, ret:%d",ret);
            //exit(-1);
        } 
    }

    return 0;
}

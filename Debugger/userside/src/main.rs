use esp32_nimble::{uuid128, BLEAdvertisementData, BLEDevice, NimbleProperties};
use esp_idf_hal::{
    can::CanDriver, gpio::{Gpio0, Input, InputPin}, peripherals, uart::{self, Uart, UartDriver}, units::Hertz, usb_serial::{config::Config, UsbSerialDriver}
};
fn bytes_to_hex(bytes: &[u8]) -> String {
    bytes.iter()
        .map(|b| format!("{:02X}", b)) // 大写 + 两位
        .collect::<Vec<_>>()
        .join(" ") // 用空格连接
}
use std::{fmt::Write, format, marker::PhantomData};
fn main() -> anyhow::Result<()> {
    /*
    目前可用的配置
    14400 19200 28800 16缓冲区
    115200 32缓冲区
     */
    esp_idf_svc::sys::link_patches();
    esp_idf_svc::log::EspLogger::initialize_default();

    let peripherals = peripherals::Peripherals::take().unwrap();
    // let computer=UartDriver::new(
    //     peripherals.uart0,peripherals.pins.gpio43,peripherals.pins.gpio44,None::<Gpio0>,None::<Gpio0>,&uart::UartConfig::default()).unwrap();
    let computer = UartDriver::new(
        peripherals.uart2,
        // peripherals.pins.gpio46,
        // peripherals.pins.gpio3,
        peripherals.pins.gpio43,
        peripherals.pins.gpio44,
        None::<Gpio0>,
        None::<Gpio0>,
        &uart::UartConfig::default()
            .parity_even()
            .baudrate(Hertz(115200)),
    )
    
    .unwrap();
    let stc = UartDriver::new(
        peripherals.uart1,
        peripherals.pins.gpio42,
        peripherals.pins.gpio1,
        None::<Gpio0>,
        None::<Gpio0>,
        &uart::UartConfig::default()
            .parity_even()
            .baudrate(Hertz(115200)),
    )
    .unwrap();

    // UsbBus::new(peripherals., unsafe { &mut EP_MEMORY });

    // let mut serial=SerialPort::new(&usebus);

    let ble_device = BLEDevice::take();
    let ble_advertising = ble_device.get_advertising();
    let server = ble_device.get_server();
    server.on_connect(|server, desc| {
        ::log::info!("Client connected: {:?}", desc);

        server
            .update_conn_params(desc.conn_handle(), 24, 48, 0, 60)
            .unwrap();

        if server.connected_count() < (esp_idf_svc::sys::CONFIG_BT_NIMBLE_MAX_CONNECTIONS as _) {
            ::log::info!("Multi-connect support: start advertising");
            ble_advertising.lock().start().unwrap();
        }
    });

    server.on_disconnect(|_desc, reason| {
        ::log::info!("Client disconnected ({:?})", reason);
    });

    let service = server.create_service(uuid128!("fafafafa-fafa-fafa-fafa-fafafafafafa"));

    // A static characteristic.
    let static_characteristic = service.lock().create_characteristic(
        uuid128!("d4e0e0d0-1a2b-11e9-ab14-d663bd873d93"),
        NimbleProperties::READ,
    );
    static_characteristic
        .lock()
        .set_value("Hello, world!".as_bytes());

    // A characteristic that notifies every second.
    let notifying_characteristic = service.lock().create_characteristic(
        uuid128!("a3c87500-8ed3-4bdf-8a39-a01bebede295"),
        NimbleProperties::READ | NimbleProperties::NOTIFY,
    );
    notifying_characteristic.lock().set_value(b"Initial value.");

    // A writable characteristic.
    let writable_characteristic = service.lock().create_characteristic(
        uuid128!("3c9a3f00-8ed3-4bdf-8a39-a01bebede295"),
        NimbleProperties::READ | NimbleProperties::WRITE,
    );
    writable_characteristic
        .lock()
        .on_read(move |_, _| {
            ::log::info!("Read from writable characteristic.");
        })
        .on_write(|args| {
            ::log::info!(
                "Wrote to writable characteristic: {:?} -> {:?}",
                args.current_data(),
                args.recv_data()
            );
        });

    ble_advertising.lock().set_data(
        BLEAdvertisementData::new()
            .name("ESP32-GATT-Server")
            .add_service_uuid(uuid128!("fafafafa-fafa-fafa-fafa-fafafafafafa")),
    )?;
    ble_advertising.lock().start()?;

    server.ble_gatts_show_local();

    let mut counter = 0;
    loop {
        let mut buf = [0u8; 64];
        let some = computer.read(&mut buf, 3_u32.into()).unwrap_or(0);
        if some != 0 {
            println!("computer-> {}", bytes_to_hex(&buf[..some]));
        }
        stc.write(&buf[..some]).unwrap();
        let mut rbuf = [0u8; 64];

        let some = stc.read(&mut rbuf, 3_u32.into()).unwrap_or(0);
        if some != 0 {
            println!("stc->{}", bytes_to_hex(&rbuf[..some]));
        }

        computer.write(&rbuf[..some]).unwrap();

        // a.write_str(&format!("Counter: {counter} {some} {}\n",String::from_utf8_lossy(&buf[..some])));

        // esp_idf_svc::hal::delay::FreeRtos::delay_ms(100);
        // notifying_characteristic

        //   .lock()
        //   .set_value(format!("Counter: {counter}").as_bytes())
        //   .notify();

        counter += 1;
    }
}

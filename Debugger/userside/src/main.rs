use anyhow::Ok;
use embedded_graphics::{
    mono_font::{ascii::FONT_6X10, MonoTextStyleBuilder},
    pixelcolor::BinaryColor,
    prelude::*,
    text::{Baseline, Text},
};
use embedded_nrf24l01::NRF24L01;
use esp_idf_hal::{
    gpio::{Gpio0, Gpio21},
    i2c::{APBTickType, I2cDriver},
    modem::Modem,
    peripherals,
    spi::{
        config::{Config, DriverConfig},
        SpiBusDriver, SpiDeviceDriver, SpiDriver,
    },
    task::{block_on, embassy_sync},
    uart::{self, UartDriver},
    units::Hertz,
};
use esp_idf_svc::{
    timer,
    wifi::{AccessPointConfiguration, AsyncWifi, Configuration, EspWifi},
};
use ssd1306::{
    mode::DisplayConfig,
    prelude::DisplayRotation,
    size::{DisplaySize128x32, DisplaySize128x64},
    I2CDisplayInterface, Ssd1306,
};
mod kcp_conn;
fn bytes_to_hex(bytes: &[u8]) -> String {
    bytes
        .iter()
        .map(|b| format!("{:02X}", b)) // 大写 + 两位
        .collect::<Vec<_>>()
        .join(" ") // 用空格连接
}
use std::{format, mem, net::UdpSocket};
async fn create_wifi(
    sysloop: &esp_idf_svc::eventloop::EspEventLoop<esp_idf_svc::eventloop::System>,
    nvs: esp_idf_svc::nvs::EspNvsPartition<esp_idf_svc::nvs::NvsDefault>,
    modem: Modem,
    timer_service: timer::EspTimerService<timer::Task>,
) -> anyhow::Result<AsyncWifi<esp_idf_svc::wifi::EspWifi>> {
    let mut wifi = AsyncWifi::wrap(
        EspWifi::new(modem, sysloop.clone(), Some(nvs))?,
        sysloop.clone(),
        timer_service,
    )?;
    let wifi_config = esp_idf_svc::wifi::Configuration::AccessPoint(AccessPointConfiguration {
        ssid: "AirriderDebug".try_into().unwrap(),
        ssid_hidden: false,
        password: "airrider666".try_into().unwrap(),
        auth_method: esp_idf_svc::wifi::AuthMethod::WPA2Personal,
        max_connections: 5,
        ..Default::default()
    });
    wifi.set_configuration(&wifi_config)?;
    wifi.start().await?;
    wifi.wait_netif_up().await?;

    Ok(wifi)
}
struct a{

}
impl a{
    fn new()->Self{
        Self {  }
    }
}
impl embedded_hal::digital::OutputPin for a{
    fn set_state(&mut self, state: embedded_hal::digital::PinState) -> Result<(), Self::Error> {
        match state {
            embedded_hal::digital::PinState::Low => self.set_low(),
            embedded_hal::digital::PinState::High => self.set_high(),
        }
    }
    
    fn set_low(&mut self) -> Result<(), Self::Error> {
        todo!()
    }
    
    fn set_high(&mut self) -> Result<(), Self::Error> {
        todo!()
    }
}

fn main() -> anyhow::Result<()> {
    esp_idf_svc::sys::link_patches();
    esp_idf_svc::log::EspLogger::initialize_default();
    let peripherals = peripherals::Peripherals::take().unwrap();
    let sysloop = esp_idf_svc::eventloop::EspSystemEventLoop::take()?;

    let nrf_spi = SpiDeviceDriver::new_single(
        peripherals.spi2,
        peripherals.pins.gpio12,
        peripherals.pins.gpio13,
        Some(peripherals.pins.gpio9),
        None::<Gpio0>,
        &DriverConfig::default(),
        &esp_idf_hal::spi::config::Config::default(),
    )?;

    let mut nrf24 = NRF24L01::new(a::new(),nrf_spi).unwrap();
    

    // let oled_i2c = I2cDriver::new(
    //     peripherals.i2c1,
    //     peripherals.pins.gpio18,
    //     peripherals.pins.gpio8,
    //     &esp_idf_hal::i2c::config::Config::default().scl_enable_pullup(false).sda_enable_pullup(false).baudrate(Hertz(400_000)),
    // )?;
    // let interface = I2CDisplayInterface::new(oled_i2c);
    // let mut display = Ssd1306::new(interface, DisplaySize128x32, DisplayRotation::Rotate0)
    //     .into_buffered_graphics_mode();

    // display.init().unwrap();
    // let text_style = MonoTextStyleBuilder::new()
    //     .font(&FONT_6X10)
    //     .text_color(BinaryColor::On)
    //     .build();

    // Text::with_baseline("Hello world!", Point::zero(), text_style, Baseline::Top)
    //     .draw(&mut display)
    //     .unwrap();

    // Text::with_baseline("Hello Rust!", Point::new(0, 16), text_style, Baseline::Top)
    //     .draw(&mut display)
    //     .unwrap();

    // display.flush().unwrap();

    // let wifi = block_on(create_wifi(
    //     &sysloop,
    //     esp_idf_svc::nvs::EspDefaultNvsPartition::take()?,
    //     peripherals.modem,
    //     timer::EspTimerService::new()?,
    // ))?;
    // mem::forget(wifi);
    // let socket = UdpSocket::bind("127.0.0.1:34254")?;
    //第一个参数是连接标识符

    Ok(())
}
// fn main() -> anyhow::Result<()> {
//     /*
//     目前可用的配置
//     14400 19200 28800 16缓冲区
//     115200 32缓冲区
//      */
//     esp_idf_svc::sys::link_patches();
//     esp_idf_svc::log::EspLogger::initialize_default();

//     let peripherals = peripherals::Peripherals::take().unwrap();
//     // let computer=UartDriver::new(
//     //     peripherals.uart0,peripherals.pins.gpio43,peripherals.pins.gpio44,None::<Gpio0>,None::<Gpio0>,&uart::UartConfig::default()).unwrap();
//     let computer = UartDriver::new(
//         peripherals.uart2,
//         // peripherals.pins.gpio46,
//         // peripherals.pins.gpio3,
//         peripherals.pins.gpio43,
//         peripherals.pins.gpio44,
//         None::<Gpio0>,
//         None::<Gpio0>,
//         &uart::UartConfig::default()
//             .parity_even()
//             .baudrate(Hertz(115200)),
//     )

//     .unwrap();
//     let stc = UartDriver::new(
//         peripherals.uart1,
//         peripherals.pins.gpio42,
//         peripherals.pins.gpio1,
//         None::<Gpio0>,
//         None::<Gpio0>,
//         &uart::UartConfig::default()
//             .parity_even()
//             .baudrate(Hertz(115200)),
//     )
//     .unwrap();

//     // UsbBus::new(peripherals., unsafe { &mut EP_MEMORY });

//     // let mut serial=SerialPort::new(&usebus);

//     loop {
//         let mut buf = [0u8; 64];
//         let some = computer.read(&mut buf, 3_u32.into()).unwrap_or(0);
//         if some != 0 {
//             println!("computer-> {}", bytes_to_hex(&buf[..some]));
//         }
//         stc.write(&buf[..some]).unwrap();
//         let mut rbuf = [0u8; 64];

//         let some = stc.read(&mut rbuf, 3_u32.into()).unwrap_or(0);
//         if some != 0 {
//             println!("stc->{}", bytes_to_hex(&rbuf[..some]));
//         }

//         computer.write(&rbuf[..some]).unwrap();

//         // a.write_str(&format!("Counter: {counter} {some} {}\n",String::from_utf8_lossy(&buf[..some])));

//         esp_idf_svc::hal::delay::FreeRtos::delay_ms(1000);

//     }
// }

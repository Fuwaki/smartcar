use std::{collections::HashMap, pin::Pin, usize};

use anyhow::{anyhow, Ok};
use embedded_hal::digital::OutputPin;
use esp_idf_hal::{modem::Modem, peripherals};
use esp_idf_svc::{
    timer,
    wifi::{AccessPointConfiguration, AsyncWifi, EspWifi},
};
use nrf24::Nrf24Conn;
use serde::{Deserialize, Serialize};
mod kcp_conn;
mod nrf24;
// mod monitor;
fn bytes_to_hex(bytes: &[u8]) -> String {
    bytes
        .iter()
        .map(|b| format!("{:02X}", b)) // 大写 + 两位
        .collect::<Vec<_>>()
        .join(" ") // 用空格连接
}
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
#[derive(Serialize,Deserialize, Debug)]
struct Package{
    info:String,
    status:bool,
    data:HashMap<String,String>,
    operate:String,
    parameter:Option<Vec<String>>
}
#[derive(PartialEq, Eq)]
enum AppStatus {
    DebugBridge,
    TransparentSerial,
}
struct App {
    info_map: HashMap<String, (String, bool)>, //value的
    write_lock: usize,
    state: AppStatus,
}
impl App {
    fn recv_remote(&mut self) {}
    fn recv_client(&mut self) {}
    fn dispatch(&mut self) {}
    #[cfg(feature = "master")]
    fn sync(&mut self) {
        self.recv_remote();
        self.recv_client();
        self.dispatch();
    }
    #[cfg(feature = "slave")]
    fn sync(&mut self) {}
    fn new() -> Self {
        let map = HashMap::from([
            ("force_x".to_string(), ("".to_string(), false)),
            ("force_y".to_string(), ("".to_string(), false)),
            ("force_z".to_string(), ("".to_string(), false)),
        ]);
        Self {
            info_map: map,
            write_lock: usize::MAX,
            state: AppStatus::DebugBridge,
        }
    }
    #[cfg(feature = "master")]
    fn init_debugger_bridge(&mut self) -> Result<(), anyhow::Error> {
        Ok(())
    }
    #[cfg(feature = "master")]
    fn init_transparent_serial(&mut self) -> Result<(), anyhow::Error> {
        Ok(())
    }
    fn transparent_serial_update(&mut self)->Result<(),anyhow::Error>{
        loop{

        }
    }
    fn set_state(&mut self, state: AppStatus) -> Result<(), anyhow::Error> {
        if self.state == state {
            return Ok(());
        }
        match state {
            AppStatus::DebugBridge => self.init_debugger_bridge(),
            AppStatus::TransparentSerial => self.init_transparent_serial(),
        }
    }

    fn run(&mut self) -> Result<(), anyhow::Error> {
        loop {
            match self.state {
                AppStatus::DebugBridge => todo!(),
                AppStatus::TransparentSerial =>self.transparent_serial_update()?,
            }
        }
        Err(anyhow!("error"))
    }
}
fn main() -> anyhow::Result<()> {
    esp_idf_svc::sys::link_patches();
    esp_idf_svc::log::EspLogger::initialize_default();
    let peripherals = peripherals::Peripherals::take().unwrap();
    let sysloop = esp_idf_svc::eventloop::EspSystemEventLoop::take()?;

    let mut nrf = Nrf24Conn::new_esp(
        peripherals.spi2,
        peripherals.pins.gpio12,
        peripherals.pins.gpio13,
        peripherals.pins.gpio9,
        peripherals.pins.gpio21,
        peripherals.pins.gpio14,
    )?;
    nrf.setup(nrf24::ROLE::SLAVE).unwrap();

    let mut app = App::new();
    app.run()?;
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

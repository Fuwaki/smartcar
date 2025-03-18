use anyhow::anyhow;
use embedded_hal::{delay::DelayNs, digital::OutputPin, spi::SpiDevice};
use esp_idf_hal::{
    delay::Delay,
    gpio::PinDriver,
    peripheral::Peripheral,
    spi::{SpiAnyPins, SpiDeviceDriver, SpiDriver},
};
use rf24::{
    radio::{prelude::*, RF24},
    PaLevel,
};
pub struct Nrf24Conn<SPI, CE, DELAY> {
    radio: RF24<SPI, CE, DELAY>,
}
pub enum ROLE {
    MASTER,
    SLAVE,
}
#[derive(Debug, thiserror::Error)]
pub enum NrfError {
    #[error("Max Send Attempts Reached")]
    SendFailure,
}

impl<CE_PIN> Nrf24Conn<
    SpiDeviceDriver<'static, SpiDriver<'static>>,
    PinDriver<'static, CE_PIN, esp_idf_hal::gpio::Output>,
    Delay,
> 
where
    CE_PIN: esp_idf_hal::gpio::OutputPin,
{
    pub fn new_esp(
        spi: impl Peripheral<P = impl SpiAnyPins> + 'static,
        sclk: impl Peripheral<P = impl esp_idf_hal::gpio::OutputPin> + 'static,
        sdo: impl Peripheral<P = impl esp_idf_hal::gpio::OutputPin> + 'static,
        sdi: impl Peripheral<P = impl esp_idf_hal::gpio::InputPin> + 'static,
        csn: impl Peripheral<P = impl esp_idf_hal::gpio::OutputPin> + 'static,
        ce: impl Peripheral<P = CE_PIN> + 'static, 
    ) -> Result<Self, anyhow::Error> {
        let spi_driver = SpiDriver::new(
            spi,
            sclk,
            sdo,
            Some(sdi),
            &esp_idf_hal::spi::config::DriverConfig::default(),
        )?;

        let spi = SpiDeviceDriver::new(
            spi_driver,
            Some(csn),
            &esp_idf_hal::spi::config::Config::default(),
        )?;

        let delay = Delay::new_default();
        let ce_pin: PinDriver<'static, _, esp_idf_hal::gpio::Output> = PinDriver::output(ce).map_err(|e| anyhow!("CE pin error: {e}"))?;

        let nrf = Nrf24Conn::new(spi, ce_pin, delay);
        Ok(nrf)
    }
}

impl<SPI, CE, DELAY> Nrf24Conn<SPI, CE, DELAY>
where
    SPI: SpiDevice,
    CE: OutputPin,
    DELAY: DelayNs,
{
    fn new(spi: SPI, ce_pin: CE, delay_impl: DELAY) -> Self {
        let radio = RF24::new(ce_pin, spi, delay_impl);
        Self { radio }
    }

    pub fn setup(&mut self, role: ROLE) -> Result<(), anyhow::Error> {
        self.radio.init().map_err(|e| anyhow!("{e:?}"))?;
        self.radio
            .set_pa_level(PaLevel::Low)
            .map_err(|e| anyhow!("{e:?}"))?;

        self.radio
            .set_dynamic_payloads(true)
            .map_err(|e| anyhow!("{e:?}"))?;
        self.radio
            .set_ack_payloads(true)
            .map_err(|e| anyhow!("{e:?}"))?;

        let address = [b"Chara", b"Frisk"];

        self.radio
            .open_rx_pipe(1, address[0])
            .map_err(|e| anyhow!("{e:?}"))?;

        self.radio
            .open_tx_pipe(address[1])
            .map_err(|e| anyhow!("{e:?}"))?;

        Ok(())
    }

    pub fn tx() {}
    pub fn rx() {}
}

use anyhow::anyhow;
use embedded_hal::{delay::DelayNs, digital::OutputPin, spi::SpiDevice};
use esp_idf_hal::{
    delay::Delay,
    peripheral::Peripheral,
    spi::{SpiAnyPins, SpiDeviceDriver, SpiDriver},
};
use rf24::{
    radio::{prelude::*, RF24},
    PaLevel,
};
struct nrf24_conn<SPI, CE, DELAY> {
    radio: RF24<SPI, CE, DELAY>,
}
enum ROLE{
    MASTER,
    SLAVE,
}
#[derive(Debug,thiserror::Error)]
enum NrfError{
    #[error("Max Send Attempts Reached")]
    SendFailure,
}
impl<SPI, CE, DELAY> nrf24_conn<SPI, CE, DELAY>
where
    SPI: SpiDevice,
    CE: OutputPin,
    DELAY: DelayNs,
{
    fn new(spi: SPI, ce_pin: CE, delay_impl: DELAY) -> Self {
        let radio = RF24::new(ce_pin, spi, delay_impl);
        Self { radio }
    }
    pub fn new_esp(
        spi: impl Peripheral<P = impl SpiAnyPins> + 'static,
        sclk: impl Peripheral<P = impl esp_idf_hal::gpio::OutputPin> + 'static,
        sdo: impl Peripheral<P = impl esp_idf_hal::gpio::OutputPin> + 'static,
        sdi: impl Peripheral<P = impl esp_idf_hal::gpio::InputPin> + 'static,
        csn: impl Peripheral<P = impl esp_idf_hal::gpio::OutputPin> + 'static,
        ce: CE,
    ) -> Result<nrf24_conn<SpiDeviceDriver<'static, SpiDriver<'static>>, CE, Delay>, anyhow::Error>
    {
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
        //TODO:这个delay可能不可以用
        let nrf: nrf24_conn<SpiDeviceDriver<'_, SpiDriver<'_>>, CE, Delay> =
            nrf24_conn::new(spi, ce, delay);
        Ok(nrf)
    }
    pub fn setup(&mut self,role:ROLE) -> Result<(), anyhow::Error> {
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

    pub fn tx(){

    }
    pub fn rx(){

    }
}

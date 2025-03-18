use embedded_graphics::{
    mono_font::{ascii::FONT_6X10, MonoTextStyleBuilder},
    pixelcolor::BinaryColor,
    prelude::Point,
    text::Text,
};
use esp_idf_hal::{
    gpio::{InputPin, OutputPin},
    i2c::I2c,
    peripheral::Peripheral,
};
use ssd1306::{
    mode::DisplayConfig, prelude::DisplayRotation, size::DisplaySize128x32, I2CDisplayInterface,
    Ssd1306,
};
use embedded_graphics::Drawable;
struct StatusMonitor {
    display: Ssd1306<
        ssd1306::prelude::I2CInterface<esp_idf_hal::i2c::I2cDriver<'static>>,
        DisplaySize128x32,
        ssd1306::mode::BufferedGraphicsMode<DisplaySize128x32>,
    >,
}
impl StatusMonitor {
    fn new<I2C: I2c>(
        i2c: impl Peripheral<P = I2C> + 'static,
        sda: impl Peripheral<P = impl InputPin + esp_idf_hal::gpio::OutputPin> + 'static,
        scl: impl Peripheral<P = impl InputPin + OutputPin> + 'static,
    ) -> anyhow::Result<Self> {
        let oled_i2c = esp_idf_hal::i2c::I2cDriver::new(
            i2c,
            sda,
            scl,
            &esp_idf_hal::i2c::config::Config::default()
                .scl_enable_pullup(false)
                .sda_enable_pullup(false),
        )?;
        let interface = I2CDisplayInterface::new(oled_i2c);
        let mut display = Ssd1306::new(interface, DisplaySize128x32, DisplayRotation::Rotate0)
            .into_buffered_graphics_mode();

        display.init().unwrap();

        Ok(Self { display })
    }
    fn update(&mut self) {
        loop {
            let text_style = MonoTextStyleBuilder::new()
                .font(&FONT_6X10)
                .text_color(BinaryColor::On)
                .build();

            Text::with_baseline("Hello world!", Point::zero(), text_style, Baseline::Top)
                .draw(&mut self.display)
                .unwrap();

            self.display.flush().unwrap();
            let mut i = 30;
            self.display.clear_buffer();
            Text::with_baseline("Ciallo", Point::zero(), text_style, Baseline::Top)
                .draw(&mut self.display)
                .unwrap();
            i += 1;
            // esp_idf_svc::hal::delay::FreeRtos::delay_ms(1);
            println!("{}", i);
            let s = format!("{}", i);
            Text::with_baseline(s.as_str(), Point::new(0, 16), text_style, Baseline::Top)
                .draw(&mut self.display)
                .unwrap();

            self.display.flush().unwrap();
        }
    }
}

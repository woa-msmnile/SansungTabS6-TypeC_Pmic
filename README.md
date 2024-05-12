# Smaung SM5705 Battery Fuel Gauge Driver

> Basd on [SurfaceDuo Battery](https://github.com/woa-project/surfacebattery).
> Reference on [HotdogBattery](https://github.com/sunflower2333/HotdogBattery/).  
* This driver is designed for the Smaung SM5705 Fuel Gauges found in the Samsung Galaxy Tab S6 LTE/WIFI. 
* This driver enables Windows to get information about battery packs used in Samsung Galaxy Tab S6 LTE/WIFI. It does not provide charging capabilities.
* **The driver still has many battery functions that have not been implemented yet, please note!**
## Future plans
* Because the USB and charging parts of Samsung Galaxy Tab S6 (WIFI) do not use PM8150B, we plan to modify the driver to control multiple I2C devices for better coordination.
## ACPI Sample

```asl
Device(SFG1)
{
    Name (_HID, "SM5705")
    Name (_UID, 1)

    Name (_DEP, Package(0x1) {
        \_SB_.IC12
    })

    Method (_CRS, 0x0, NotSerialized) {
        Name (RBUF, ResourceTemplate () {
            I2CSerialBus(0x71,, 100000, AddressingMode7Bit, "\\_SB.IC12",,,,)
            GpioInt(Level, ActiveBoth, Exclusive, PullNone, 0, "\\_SB.GIO0") {37}  // IRQ
        })
        Return (RBUF)
    }
}
```
## Acknowledgements
* [Gustave Monce](https://github.com/gus33000)
* [WOA-Project](https://github.com/WOA-Project)
* [WOA-Msmnile](https://github.com/woa-msmnile)
* [sunflower2333](https://github.com/sunflower2333)
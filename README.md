# vroomm-4k-cryostat

UICS instrument-control daemon for the VROOMM 4K cryostat. It polls the
cryostat devices through `liblabdev` (Sunpower cooler `D_Sunpower`, Lakeshore
336 `D_Lakeshore`, MKS 972B gauge) and exposes their readings as Prometheus
gauges. Commands are defined in `config/cmd.cfg` and implemented in
`src/vroomm4k_standard_cmd.cpp`; all of them run in the `CONFIG` state.

Setting commands do serial/socket I/O on the command thread, so the handler
disables the device in the DeviceManager (about 1 s) for the duration of the
exchange and re-enables it afterwards. Every setting is read back from the
instrument, and the command fails if the controller does not report the
requested value.

## `cooler` (Sunpower MT controller)

| Command | Effect |
|---|---|
| `cooler set <ON\|OFF>` | Start or stop the cooler. |
| `cooler target <K>` | Set the temperature target (`TTARGET=<K>`, kelvin). The controller keeps it in non-volatile memory and the command is user-locked on the controller. |
| `cooler vibe <ON\|OFF> [-feature <WAVE\|HEAD>]` | Active vibration cancellation (default feature `WAVE`). |
| `cooler -vibe` | Read the vibration state. |
| `cooler -status` | Print cooler state, powers, target/reject/coldhead temperatures and the active errors. |

### Errors

`cooler -status` prints the controller's `ERROR` flags in binary, for example
`ERRORS (binary):  00001010`.

| Bits | Meaning |
|---|---|
| `00000001` | High reject temperature |
| `00000010` | Low reject temperature |
| `10000000` | Over current |
| `11111111` | Invalid configuration |

The errors are also exported as Prometheus gauges on `D_Sunpower`: `Error`
(the bitmask as an integer, 0 = no error), `Error_High_Reject_Temp`,
`Error_Low_Reject_Temp`, `Error_Over_Current` and `Error_Invalid_Config`
(1 = active). When `Error_Invalid_Config` is 1 the other three flag gauges
read 0.

## `lakeshore` (Lakeshore 336)

```
lakeshore output <1-4> [range <r>] [rate <K/min>] [ramp <ON|OFF>] [setpoint <val>]
lakeshore -status
```

`output` is required for any setting. Several settings can be given in one
call; they are applied in the order range, ramp/rate, setpoint, and the first
failure stops the command. All arguments are validated before the instrument
is touched.

| Argument | Meaning |
|---|---|
| `setpoint <val>` | `SETP`: setpoint of the output's control loop, in the units of its control input (kelvin for a temperature loop). Outputs 1-4; on 3-4 it is only used in Warmup mode. |
| `ramp <ON\|OFF>` | Enable/disable setpoint ramping (outputs 1-2). Keeps the current rate. |
| `rate <K/min>` | Ramp rate, 0.1 to 100 K/min (outputs 1-2). Keeps the current on/off state. Rate 0 is rejected because the controller treats it as "ramping off"; use `ramp OFF`. |
| `range <r>` | Heater range. Outputs 1-2: `OFF`, `LOW`, `MEDIUM`, `HIGH` (or 0-3). Outputs 3-4: `OFF`, `ON` (or 0-1). |
| `-status` | Setpoint, ramp state/rate and heater range of outputs 1 and 2. |

Examples:

```
lakeshore output 1 setpoint 77
lakeshore output 1 rate 2.5 ramp ON setpoint 120   # ramp to 120 K at 2.5 K/min
lakeshore output 1 range HIGH
lakeshore output 1 ramp OFF
lakeshore -status
```

When ramping is on, a new setpoint is approached at the ramp rate, so set the
rate and enable ramping before changing the setpoint (the combined form above
does this).

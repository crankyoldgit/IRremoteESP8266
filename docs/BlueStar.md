# Blue Star IE318YKU

`BLUESTAR` / `IRBluestarAc` implements the IE318YKU protocol. It is separate
from `BLUESTARHEAVY`. The remote model number is unknown.

Status: **experimental**. Real ESP32 `IRrecvDumpV2` captures establish the
packet layout and controls below. Receiver decoding and generated waveforms
are tested in software. A demodulated capture cannot establish carrier
frequency; **38 kHz is the initial replay frequency**, with hardware sender
validation still required.

## Wire and storage layout

The wire contains **91 data bits** in two sections, with 185 durations:

* Main: 80 bits, MSB first; ten bytes including the checksum. Approximately
  1000 us marks, 580 us zero spaces, 2600 us one spaces.
* Separator: 1000 us mark and 8950 us space.
* Secondary: 11 bits, MSB first; approximately 660 us marks, 3700 us zero
  spaces, 6500 us one spaces, then a final 660 us mark.

`kBluestarStateLength` is **12 storage bytes**. Bytes 0-9 hold the main packet.
Bytes 10-11 hold the 11-bit secondary command, right-aligned, most significant
byte first. Their upper five storage bits are zero and are not transmitted.
For example, `10111000100` is stored as `05 C4`. `decode_results::bits` is 91;
`resultToSourceCode()` prints all twelve storage bytes, suitable for replay.

The main checksum is `~sum(bytes[0..8]) & 0xFF` at byte 9. It excludes the
secondary. `getRaw()` recomputes it; low-level `sendBluestar()` accepts the
checksum supplied by the caller.

## Observed fields

Bit zero means the least significant bit of a byte.

| Main byte / bits | Meaning | Evidence |
| --- | --- | --- |
| 0 | Signature `33` | All captured messages |
| 1 / 0-3 | Mode: Fan-only 1, Auto 2, Dry 4, Cool 8 | Complete mode cycle |
| 1 / 5-7 | Fan: High 1, Medium 2, Low 4, Auto 7 (shifted values) | Complete fan cycle |
| 1 / 5-7 = 0 | Convertible Turbo fan code; displayed as High | Convertible cycle and user observation |
| 2 / 0 | Horizontal Swing; zero = On | Controlled reversible toggle |
| 2 / 1 | Vertical Swing; zero = On | Controlled reversible toggle |
| 2 / 3 | Light; zero = On | Controlled reversible toggle |
| 2 / 4 | Standalone Turbo; one = On | Controlled reversible toggle |
| 2 / 6 | Sleep; one = On | Controlled reversible toggle |
| 2 / 7 | Power; one = On | Three power cycles |
| 3 / 0-4 | Stored Celsius temperature, 16-30 in Cool | Temperature sweep and controlled captures |
| 3 / 5 | Energy Save; one = On | Isolated disabling transition |
| 4, 5, 7 | Clock-related values/counters; full interpretation unresolved | Controlled clock edits |
| 6 | Clock minute | Explicit 00, 01, 59 minute captures |
| 8 / 0-2 | Capacity: Turbo/110%=1, 80%=2, 60%=3, 40%=4, Default/100%=5 | Complete capacity cycle |
| 9 | Complement-sum checksum | All captured messages |

All remaining bits are preserved rather than assigned meanings. Initial
temperature captures described both swings as On, but their bit values
conflict with the subsequent independent controlled tests. The implementation
uses the latter evidence; the original labels have not been silently changed.

## Secondary commands

The secondary is variable. Identical main packets have been captured with
different secondary values. On and Off directions of a feature use the same
secondary in the tested cases.

| Action | Secondary value |
| --- | --- |
| Mode | `5C0` |
| Temp Down / ordinary temperature state | `5C4` |
| Either swing axis | `5C8` |
| Power | `5CD` |
| Energy Save | `7C1` |
| Standalone Turbo | `7C9` |
| Sleep | `7CC` |
| Fan | `7D0` |
| Light or Convertible | `000` |

One automatically labeled Temp Down baseline had the Swing secondary. Its
actual physical baseline button was not independently recorded, so this
exception remains in the research evidence. Temperature increment's secondary
has not been captured separately. The native class selects the observed
temperature command for `setTemp()`; hardware tests should include increasing
as well as decreasing temperature. `setCommand()` and `setRaw()` permit
preserving any other eleven-bit secondary without inventing its meaning.

## State behavior

* Power-button captures below 24 C clamp the setpoint to 24; a 25 C power
  cycle retains 25. `setPower()` mirrors that behavior. A subsequent `setTemp()`
  can select 16-23 while On; ordinary On packets at those temperatures were
  captured and are valid.
* Energy Save On selects 24 C and Low fan. Disabling retains those values.
* Standalone Turbo retains the stored base temperature/fan in its packet.
  The remote temporarily displays 16 C/High and restores its prior values
  when Turbo is disabled. `getTemp()` / `getFan()` expose stored values;
  `getEffectiveTemp()` / `getEffectiveFan()` and `toCommon()` include this
  override. It is independent of Convertible Turbo.
* Convertible changes byte 8 and the fan code. Codes 1/2/3/4 select native
  fan codes 0/1/2/4 respectively; temperature is unchanged. Default restores
  a fan remembered by this class before a locally set override. A raw active
  capacity capture does not reveal the earlier user fan preference.
* Fan-only has no temperature control on the physical remote. Its captured
  hidden temperature is 24; the native class uses that value on entry and
  ignores temperature setters while in Fan-only.
* The remote restored Cool 16 C/Low after the tested mode cycle, and other
  modes transmitted Dry 27/Low, Fan-only 24/High, Auto 24/Auto. These can be
  remembered settings rather than universal defaults. The class does not
  invent per-mode remote history: select desired temperature/fan explicitly.
* Clock/timer bytes survive setters and raw imports. Timer support was
  excluded from this investigation. Full clock/hour encoding is incomplete;
  there is no clock/timer setter and common `clock` stays unset. Byte 6 is
  available through `getClockMinute()`. Resolved AC controls do not require
  changing the opaque time fields.

The common API can send/decode ordinary controls, including standalone Turbo,
Energy Save, Light, Sleep, and independent swings. Use the native class for
capacity, secondary commands, raw imports, and stored/effective values. Common
sending uses the captured Power secondary with the complete desired state;
hardware verification of combined-setting changes is still needed.

## ESP32 replay and control

`examples/ControlBluestarAC` sends on **GPIO 15** at 115200 baud and transmits
only when a serial command requests it. Adjust `kIrLed` for other wiring.

```sh
pio run -d examples/ControlBluestarAC -e esp32dev -t upload --upload-port /dev/ttyUSB0
pio device monitor --port /dev/ttyUSB0 --baud 115200
```

First replay a captured Power On message. In the serial monitor:

```text
load 3388821812922AA4053305CD
send
```

Then test generated changes one at a time:

```text
temp 16
fan high
horizontal off
vertical on
light off
light on
turbo on
turbo off
capacity 110
capacity 80
capacity 60
capacity 40
capacity 100
econo on
econo off
sleep on
sleep off
off
on
```

Confirm the actual unit responds and report any mismatches. `state` prints
current state/storage without transmitting. `load` validates the checksum and
does not transmit until `send`. Use `mode auto|cool|dry|fan` and explicit
temperature/fan commands for mode testing. Research capture tooling/logs
remain outside the repository; representative real raw arrays are retained
in `test/ir_Bluestar_test_data.h` for reproducible regression tests.

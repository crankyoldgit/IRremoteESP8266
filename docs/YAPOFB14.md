# Gree YAPOFB14: basic control

Use `IRGreeAC ac(pin, YAPOFB14)` or `GREE` / model 4 with `IRac`.
`IRac::strToModel()` also accepts `YAP0FB14`.

This model sends two 8-byte groups (128 bits) and supports power, mode,
temperature, five fan speeds, Quiet, Turbo, Light, vertical/horizontal swing
and ordinary sleep.

Fan 0 is Auto; 1–5 increase the speed. Quiet selects fan 1, Turbo selects fan 5,
and setting the fan clears both. `setSleep(true)` selects Sleep 1 with Quiet/low
fan. IRac uses sleep as an on/off setting. Heat mode keeps the remote's default
auxiliary-heat setting and handles its shared X-Fan bit separately from Clean.

Pass `getStateLength()` with `getRaw()` when copying or sending the state.
`setRaw(data, length)` accepts 8 or 16 bytes; its default remains 8 bytes.
Timer calls are inactive for this model.

Reception needs a timeout longer than the approximately 40 ms inter-group gap
and room for 279 pulse durations; a 512-entry buffer and 50 ms timeout suffice.
128-bit Gree and Kelvinator overlap, so automatic decoding retains Kelvinator
priority. For a known Gree remote, use
`decodeGree(&result, kStartOffset, kGreeExtendedBits)` explicitly.

Regression tests use original remote captures and state transitions.
Earlier hardware testing confirmed ordinary control and display on/off.

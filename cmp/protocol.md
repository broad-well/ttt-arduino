# Ttt-Arduino Board/Host communication protocol

This protocol provides the ability for the board and the host to communicate via Serial.

## Message format

`{protocol}:{data...}`; data is split by `;`.

**Example: `board:OX ;X X;OO `**

## Protocols

### `board`

Transmits the state of the entire board. Usually used from board to host.

### `cell`



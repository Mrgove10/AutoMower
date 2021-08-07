# Automower

This Project

[![PlatformIO CI](https://github.com/Mrgove10/AutoMower/actions/workflows/main.yml/badge.svg)](https://github.com/Mrgove10/AutoMower/actions/workflows/main.yml)
[![CodeFactor](https://www.codefactor.io/repository/github/mrgove10/automower/badge)](https://www.codefactor.io/repository/github/mrgove10/automower)

## Table of contents

- [Automower](#automower)
  - [Table of contents](#table-of-contents)
  - [Communicating with the Mower](#communicating-with-the-mower)
    - [IDLE](#idle)
    - [DOCKED](#docked)
    - [MOWING](#mowing)
    - [TO_BASE](#to_base)
    - [FROM_BASE](#from_base)
    - [ERROR](#error)
    - [TEST](#test)
    - [OTA](#ota)
    - [DBG_VERBOSE](#dbg_verbose)
    - [DBG_DEBUG](#dbg_debug)
    - [BBG_INFO](#bbg_info)
    - [TEST_MOTOR](#test_motor)
    - [TEST_CUTMOTOR](#test_cutmotor)
  - [License](#license)
  - [Contribution](#contribution)

## Communicating with the Mower

Communication is done with MQTT, the folowing topics are used :

### IDLE

**Description** : This is used to put the mower in the idle state. In this state the mower is doing nthing and wait for the next command.

**Topic** : `IDLE`

**Message** :

```json
{
  "Command":"STATE_CHANGE",
  "Val1":"IDLE"
}
```

### DOCKED

**Description** : This command send the miwer back to the dock

**Topic** : `DOCKED`

**Message** :

```json
{
  "Command":"STATE_CHANGE",
  "Val1":"DOCKED"
}
```

### MOWING

### TO_BASE

### FROM_BASE

### ERROR

### TEST

### OTA

### DBG_VERBOSE

### DBG_DEBUG

### BBG_INFO

### TEST_MOTOR

### TEST_CUTMOTOR

## License

This project is under the MIT License, see [here](LICENSE.md)

## Contribution

You can contribute by submitting a PR
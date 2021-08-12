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

## Sending Commands and values to the Mower

Communication is done through MQTT, and the following topics are used:

### Automower/Command

 The messages in this topic are in JSON format and should follow this structure:

 ```json
{
  "Command":"<Command>",
  "Val1":"<Command_Dependant_String_1",
  "Val2":"<Command_Dependant_String_2"
}
```
The list of Commands and their expected associated values are as describes below:

 channel following topics are used :

#### STATE_CHANGE

**Description** : This command is used to put the mower in a given state. 

In this state the mower is doing nthing and wait for the next command.

**Command** : `STATE_CHANGE`

**Val1** : Possible values for Val1 are:

`IDLE` : In this state the mower is doing nothing and waits for the next command. No Val2 value expected (any value sent will be ignored)

`DOCKED` : In this state the mower is docked. No Val2 value expected (any value sent will be ignored)

`MOWING` : In this state the mower is doing it's primary work....mowing the lawn ! No Val2 value expected (any value sent will be ignored)

`TO_BASE` : In this state the mower is returning to its base/charging station. No Val2 value expected (any value sent will be ignored)

`FROM_BASE` : In this state the mower is leaving its base/charging station and going to the mowing zone. No Val2 value expected (any value sent will be ignored)

`ERROR` : This is the state when the Mower has identified an error condition and is stopped and waiting for an acknowledgement from the user. This command is not for operational use and only for testing purposes. No Val2 value expected (any value sent will be ignored)

`ACKNOWLEDGE` : This is the command to acknoledge an error. Upon acknowledgement, the mower will retun to Idle state. Acknowledgement can also be performed directly on the mower's HMI. No Val2 value expected (any value sent will be ignored)

**Message Example** :

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

## Receiving informationfrom the Mower

## License

This project is under the MIT License, see [here](LICENSE.md)

## Contribution

You can contribute by submitting a PR
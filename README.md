# Automower

[![PlatformIO CI](https://github.com/Mrgove10/AutoMower/actions/workflows/mower.yml/badge.svg)](https://github.com/Mrgove10/AutoMower/actions/workflows/mower.yml)
[![WebApp CI](https://github.com/Mrgove10/AutoMower/actions/workflows/webapp.yml/badge.svg)](https://github.com/Mrgove10/AutoMower/actions/workflows/webapp.yml)
[![CodeFactor](https://www.codefactor.io/repository/github/mrgove10/automower/badge)](https://www.codefactor.io/repository/github/mrgove10/automower)

**Note: This project is a work in progress !!**

## Table of contents

- [Automower](#automower)
  - [Table of contents](#table-of-contents)
  - [Sending Commands and values to the Mower](#sending-commands-and-values-to-the-mower)
    - [Automower/Command](#automowercommand)
      - [Mower State changes](#mower-state-changes)
      - [Calibration](#calibration)
      - [Over The Air program update (OTA)](#over-the-air-program-update-ota)
      - [ESP restart](#esp-restart)
      - [Change Trace Level](#change-trace-level)
      - [Parameter value change](#parameter-value-change)
      - [Test & Debug commands](#test--debug-commands)
        - [Stop/activate sending of raw perimeter signal read task](#stopactivate-sending-of-raw-perimeter-signal-read-task)
        - [Stop/activate sending of processed perimeter code detection task](#stopactivate-sending-of-processed-perimeter-code-detection-task)
        - [Stop/activate sending of perimeter wire tracking control](#stopactivate-sending-of-perimeter-wire-tracking-control)
        - [Forward mower test move](#forward-mower-test-move)
        - [Reverse mower test move](#reverse-mower-test-move)
        - [Turn mower test move](#turn-mower-test-move)
        - [Motion motor Test](#motion-motor-test)
        - [Cutting motor Test](#cutting-motor-test)
        - [TEST_STOP](#test_stop)
        - [Test](#test)
  - [Receiving information from the Mower](#receiving-information-from-the-mower)
  - [Tasks](#tasks)
  - [Website](#website)
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

#### Mower State changes

**Description** : This command is used to put the mower in a given state.

In this state the mower is doing nthing and wait for the next command.

**Command** : `STATE_CHANGE`

**Val1** : Possible values for Val1 are:

- `IDLE` : In this state the mower is doing nothing and waits for the next command.

- `DOCKED` : In this state the mower is docked.

- `MOWING` : In this state the mower is doing it's primary work....mowing the lawn !

- `TO_BASE` : In this state the mower is returning to its base/charging station.

- `FROM_BASE` : In this state the mower is leaving its base/charging station and going to the mowing zone.

- `ERROR` : This is the state when the Mower has identified an error condition and is stopped and waiting for an acknowledgement from the user. This command is not for operational use and only for testing purposes.

- `ACKNOWLEDGE` : This is the command to acknoledge an error. Upon acknowledgement, the mower will retun to `IDLE` state. Acknowledgement can also be performed directly on the mower's HMI.

- `TEST` : This is the command to trigger the mower's startup test sequence. At the end, the mower will retun to `IDLE` state.

**Val2** : 

- For `MOWING` state, possible values for Val1 are:
  - `0` for Random mowing.
  - `1` for spiral mowing clockwise 
  - `2` for spiral mowing counter clockwise
- No Val2 value expected for other states (any value sent will be ignored)

**Message Example** :

Set mower to idle state:

```json
{
  "Command":"STATE_CHANGE",
  "Val1":"IDLE"
}
```
Set mower to clockwise spiral mowing:

```json
{
  "Command":"STATE_CHANGE",
  "Val1":"MOWING",
  "Val2":"1"
}
```

#### Calibration

**Description** : This command triggers the perimeter signal offset calibration

**Command** : `CALIBRATION`

**Val1** : No Val1 value expected for this command (any value sent will be ignored)

**Val2** : No Val2 value expected for this command (any value sent will be ignored)

**Message Example** :

```json
{
  "Command":"CALIBRATION"
}
```

#### Over The Air program update (OTA)

**Description** : This command places the mower in a condition ready to receive an OTA upate. This stops the mower (motor stops), suspsends the MQTT communications and suspends mower RTOS tasks. At end of successful OTA, the mower program resets. If OTA is not performed within a preconfigure duration (¬ 3 minutes), normal mower functions resume and the mower is placed in `IDLE` state.

**Command** : `OTA`

**Val1** : No Val1 value expected for this command (any value sent will be ignored)

**Val2** : No Val2 value expected for this command (any value sent will be ignored)

**Message Example** :

```json
{
  "Command":"OTA"
}
```

#### ESP restart

**Description** : This command causes the ESP to reset itself. This stops the mower (motor stops), saves values to EEPROM and triggers a system reboot.

**Command** : `RESTART`

**Val1** : No Val1 value expected for this command (any value sent will be ignored)

**Val2** : No Val2 value expected for this command (any value sent will be ignored)

**Message Example** :

```json
{
  "Command":"RESTART"
}
```

#### Change Trace Level

**Description** : This command enables to change the current trace visualisation level to the specified level. Any message "below" the specified level are no longer displayed on the console (USB serial port) or the Telnet console.

**Command** : `DEBUG`

**Val1** : Val1 contains the level to be set. Possible values for Val1 are:

- `VERBOSE` : At this "lowest" level, all trace messages are displayed.

- `DEBUG` : At this level, VERBOSE level mesages are not displayed, all others are.

- `INFO` : At this level, VERBOSE and DEBUG level mesages are not displayed, all others are (INFO and ERROR).

**Note** INFO and ERROR messages cannot be hidden.

**Val2** : No Val2 value expected for this command (any value sent will be ignored)

**Message Example** :

```json
{
  "Command":"DEBUG",
  "Val1":"DEBUG"
}
```

#### Parameter value change

**Description** : This command enables to send a new parameter value to the mower. If the value is stored in EEPROM (most of them are), the EEPROM is updated/saved.

**Command** : `PARAMETER`

**Val1** : Val1 contains the string code of the parameter. Possible values are:

- `PerimTtrkngKp` : PID control Kp (proportional )parameter for perimeter wire tracking,

- `PerimTtrkngKi` : PID control Ki (integral) parameter for perimeter wire tracking,

- `PerimTtrkngKd` : PID control Kd (derivative) parameter for perimeter wire tracking,

- `PerimTtrkSetPt` : PID control setpoint for perimeter tracking,

- `PerimLostThresld` : Perimeter wire signal lost threshold (perimeter wire cut or sender stopped),

- `PerimTtrkLowThresld` : Perimeter wire signal too low for tracking threshold (mower no longer "over" perimeter wire).

**Note** : Incorrectly spelt or unknown parameter codes are rejected.

**Val2** : Val2 contains the parameter value as a decimal point (float) value. If the paramater is an integer value and a float value is sent, it will be truncated.

**Message Example** : set perimeter wire tracking PID Ki parameter to 1.52 :

```json
{
  "Command":"PARAMETER",
  "Val1":"PerimTtrkngKi",
  "Val2":"1.52",
}
```

#### Test & Debug commands

##### Stop/activate sending of raw perimeter signal read task

**Description** : This command enables to start and stop the sending over MQTT of the raw perimeter signal read task detailled values used for plotting //**ONLY FOR TEST PURPOSES**//

**Command** : `START_MQTT_GRAPH_RAW_DEBUG` or `STOP_MQTT_GRAPH_RAW_DEBUG`

**Val1** : No Val1 value expected for this command (any value sent will be ignored)

**Val2** : No Val2 value expected for this command (any value sent will be ignored)

**Message Example** :

```json
{
  "Command":"START_MQTT_GRAPH_RAW_DEBUG"
}
```

##### Stop/activate sending of processed perimeter code detection task

**Description** : This command enables to start and stop the sending over MQTT of the processed perimeter code detection task values used for plotting //**ONLY FOR TEST PURPOSES**//

**Command** : `START_MQTT_GRAPH_DEBUG` or `STOP_MQTT_GRAPH_DEBUG`

**Val1** : No Val1 value expected for this command (any value sent will be ignored)

**Val2** : No Val2 value expected for this command (any value sent will be ignored)

**Message Example** :

```json
{
  "Command":"START_MQTT_GRAPH_DEBUG"
}
```

##### Stop/activate sending of perimeter wire tracking control

**Description** : This command enables to start and stop the sending over MQTT of the perimeter wire tracking PID control values used for plotting //**ONLY FOR TEST PURPOSES**//

**Command** : `START_MQTT_PID_GRAPH_DEBUG` or `STOP_MQTT_PID_GRAPH_DEBUG`

**Val1** : No Val1 value expected for this command (any value sent will be ignored)

**Val2** : No Val2 value expected for this command (any value sent will be ignored)

**Message Example** :

```json
{
  "Command":"STOP_MQTT_PID_GRAPH_DEBUG"
}
```

##### Forward mower test move

**Description** : This command enables to request for a forward move as specified in the associated values. //**ONLY FOR TEST PURPOSES - NO COLISION DETECTION PERFORMED**//

**Command** : `TEST_FORWARD`

**Val1** : Val1 contains the Speed at which the move is to be performed (range [0, 100%])

**Val2** : Val2 contains the duration (in seconds) of the move

**Message Example** : forward 5 seconds @ 90% speed :

```json
{
  "Command":"TEST_FORWARD",
  "Val1":"90",
  "Val2":"5",
}
```

##### Reverse mower test move

**Description** : This command enables to request for a reverse move as specified in the associated values. //**ONLY FOR TEST PURPOSES - NO COLISION DETECTION PERFORMED**//

**Command** : `TEST_REVERSE`

**Val1** : Val1 contains the Speed at which the move is to be performed (range [0, 100%])

**Val2** : Val2 contains the duration (in seconds) of the move

**Message Example** : Reverse 10 seconds @ 75% speed :

```json
{
  "Command":"TEST_REVERSE",
  "Val1":"75",
  "Val2":"10",
}
```

##### Turn mower test move

**Description** : This command enables to request for a turn as specified in the associated values. //**ONLY FOR TEST PURPOSES - NO COLISION DETECTION PERFORMED**//

**Command** : `TEST_TURN`

**Val1** : Val1 contains the angle to turn. Positive angle for right turn, negative for left turn. Angle may be greater than 360 (positive or negative)

**Val2** : Val2 contains an indicator (1=active, 0=inactive) that indicates that the turn should be "on the spot" (inner wheel reverses while outter wheel goes forward) or not (inner wheel remains stopped). If Val2 is omitted, val2 is considered to be 0.

**Message Example** : turn righ 90 degrees, on the spot :

```json
{
  "Command":"TEST_TURN",
  "Val1":"90",
  "Val2":"1",
}
```

##### Motion motor Test

**Description** : not sure... to be checked //**ONLY FOR TEST PURPOSES**//

**Command** : `TEST_MOTOR`

**Val1** : No Val1 value expected for this command (any value sent will be ignored)

**Val2** : No Val2 value expected for this command (any value sent will be ignored)

**Message Example** :

```json
{
  "Command":"TEST_MOTOR"
}
```

##### Cutting motor Test

**Description** : not sure... to be checked //**ONLY FOR TEST PURPOSES**//

**Command** : `TEST_CUTMOTOR`

**Val1** : No Val1 value expected for this command (any value sent will be ignored)

**Val2** : No Val2 value expected for this command (any value sent will be ignored)

**Message Example** :

```json
{
  "Command":"TEST_CUTMOTOR"
}
```

##### TEST_STOP

**Description** : This command stops the mower during tests //**ONLY FOR TEST PURPOSES**//

**Command** : `TEST_STOP`

**Val1** : No Val1 value expected for this command (any value sent will be ignored)

**Val2** : No Val2 value expected for this command (any value sent will be ignored)

**Message Example** :

```json
{
  "Command":"TEST_STOP"
}
```

##### Test

**Description** : not sure... to be checked

**Command** : `TEST`

**Val1** : No Val1 value expected for this command (any value sent will be ignored)

**Val2** : No Val2 value expected for this command (any value sent will be ignored)

**Message Example** :

```json
{
  "Command":"TEST"
}
```

## Receiving information from the Mower

// TO DO

## Tasks

// TO DO

## Website

For developping the website you need node.js installed (version 16+ recommended)

To launch the website  :

``` bash
cd APP
npm install
npm run serve
```

You can now go to <http://locahost:8080>

## License

This project is under the MIT License, see [here](LICENSE.md)

## Contribution

You can contribute by submitting a Pull Request

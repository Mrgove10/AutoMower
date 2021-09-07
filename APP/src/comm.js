import { mowerStates } from './jsons/states'
// import { connect } from 'mqtt/dist/mqtt'
import { updatevalue } from './utils/utils'

const ws = new WebSocket('ws://192.168.1.38:1880/AM/ws')

ws.onmessage = function (event) {
    event = JSON.parse(event.data)
    updatevalue('debugData', JSON.stringify(event))
    console.log(event)
    updatevalue('test', event.BatVolt)
    updatevalue('stateIcon', mowerStates[event.State].icon)
    updatevalue('stateDisplay', mowerStates[event.State].humanName)
}

/**
 * Send an MQTT command
 * @param {*} command
 * @param {*} val1
 * @param {*} val2
 * @returns
 */
// export function sendCommand(command, val1 = null, val2 = null) {
//     if (!command) {
//         console.error('No command inputed')
//         return
//     }
//     console.log('Sending command')
//     const data = {
//         Command: command,
//         Val1: val1,
//         Val2: val2
//     }
//     console.log(data)
//     client.publish('Automower/Command', JSON.stringify(data))
// }

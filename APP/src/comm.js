import { getConfig } from './config'
import { connect } from 'mqtt/dist/mqtt'

const client = connect('mqtt://' + getConfig().mqttUrl)

/**
 * Send an MQTT command
 * @param {*} command
 * @param {*} val1
 * @param {*} val2
 * @returns
 */
export function sendCommand (command, val1 = null, val2 = null) {
    if (!command) {
        console.error('No command inputed')
        return
    }
    console.log('Sending command')
    const data = {
        Command: command,
        Val1: val1,
        Val2: val2
    }
    console.log(data)
    client.publish('Automower/Command', JSON.stringify(data))
}

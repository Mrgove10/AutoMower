import { getConfig, getSchedule, APIUrl } from './config'
import { mowerStates } from './jsons/states'
import { createTable, addSchedule } from './schedule'
const axios = require('axios').default

// Entry point
async function main () {
    getConfig()
    getSchedule()
    createTable()
    await addSchedule()
}

main()

window.start = function () {
    console.log('Sending Start')
    // sendCommand('STATE_CHANGE', 'MOWING')
}

window.stop = async function () {
    console.log('Sending Stop')
    await axios.get(APIUrl + '/STOP')
    // sendCommand('STATE_CHANGE', 'IDLE')
}

window.setStateIcon = function (State) {
    document.getElementById('stateDisplay').innerHTML = `${mowerStates[State].icon} ${mowerStates[State].humanName}`
}

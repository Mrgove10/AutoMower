import { sendCommand } from './comm'
import { getConfig, getSchedule, mowerStates } from './config'
import { createTable } from './schedule'

// Entry point
getConfig()
getSchedule()
createTable()
getSchedule()

window.start = function () {
    console.log('Sending Start')
    sendCommand('STATE_CHANGE', 'MOWING')
}

window.stop = function () {
    console.log('Sending Stop')
    sendCommand('STATE_CHANGE', 'IDLE')
}

window.setStateIcon = function (State) {
    document.getElementById('stateDisplay').innerHTML = `${mowerStates[State].icon} ${mowerStates[State].humanName}`
}

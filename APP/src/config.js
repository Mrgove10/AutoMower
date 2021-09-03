const axios = require('axios').default

export const mowerStates = {
    idle: {
        humanName: 'Idle',
        icon: ''
    },
    docked: {
        humanName: 'Docked',
        icon: '<i class="fas fa - home"></i>'
    },
    mowing: {
        humanName: 'Mowing',
        icon: ''
    },
    going_to_base: {
        humanName: 'Going to the dock',
        icon: ''
    },
    leaving_base: {
        humanName: 'Leaving dock',
        icon: ''
    },
    error: {
        humanName: 'Error',
        icon: ''
    },
    test: {
        humanName: 'test',
        icon: ''
    }
}

export const days = [
    'Monday',
    'Tuesday',
    'Wednesday',
    'Thursday',
    'Friday',
    'Saturday',
    'Sunday'
]

export const APIUrl = 'http://192.168.1.38:1880/AM'
let config = {}
let schedule = {}

export async function getConfig () {
    if (config !== {}) {
        console.log('Getting config from server')
        const res = await axios.get(APIUrl + '/config')
        config = res.data
    }
    return config
}

export async function saveConfig () {
    console.log('Saving config')
    const res = await axios.post(APIUrl + '/config', JSON.parse(config))
    console.log(JSON.parse(res.data))
}

export async function getSchedule () {
    if (schedule !== {}) {
        console.log('Getting Schedule from server')
        const res = await axios.get(APIUrl + '/schedule')
        schedule = res.data
    }
    return schedule
}

export async function saveSchedule (schedule) {
    console.log('Saving Schedule')
    const res = await axios.post(APIUrl + '/schedule', JSON.parse(schedule))
    console.log(JSON.parse(res.data))
}

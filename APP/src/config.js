const axios = require('axios').default

export const APIUrl = 'http://192.168.1.38:1880/AM'
let config = {}
let schedule = {}

/**
 * get the config for the server
 * If the config has already been taken from the server,use the local copy
 * @returns
 */
export async function getConfig () {
    if (config !== {}) {
        console.log('Getting config from server')
        const res = await axios.get(APIUrl + '/config')
        config = res.data
    }
    return config
}

/**
 * Save the configuration on the server
 * @param {*} config
 */
export async function saveConfig (config) {
    console.log('Saving config')
    const res = await axios.post(APIUrl + '/config', JSON.parse(config))
    console.log(JSON.parse(res.data))
}

/**
 * Get the config from the server
 * @returns json
 */
export async function getSchedule () {
    if (schedule !== {}) {
        console.log('Getting Schedule from server')
        const res = await axios.get(APIUrl + '/schedule')
        schedule = res.data
    }
    return schedule
}

/**
 * Save the schedule to the server
 * If the schedule has already been taken from the server,use the local copy
 * @param {*} schedule
 */
export async function saveSchedule (schedule) {
    console.log('Saving Schedule')
    const res = await axios.post(APIUrl + '/schedule', JSON.parse(schedule))
    console.log(JSON.parse(res.data))
}

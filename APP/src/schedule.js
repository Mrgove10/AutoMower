import { getSchedule } from './config'
import { days } from './jsons/days'

const d = new Date()

/**
 * Create the table structure for the schedule page
 */
export function createTable () {
    // table header
    days.forEach((day) => {
        document.getElementById('tableheader').innerHTML = document.getElementById('tableheader').innerHTML + `<th width="14.28%">${day}</th>`
    })

    // table body
    for (let i = 1; i <= 24; i++) {
        const current = document.getElementById('tablebody').innerHTML
        let strDay = `<tr><td>${i}:00</td>`
        days.forEach((day) => {
            strDay = strDay + `<td id="${i}_${day}" class="" rowspan="1"></td>`
        })
        document.getElementById('tablebody').innerHTML = current + strDay + '</tr>'
    }
}

/**
 * Add the schedule colors in the table
 */
export async function addSchedule () {
    const schedule = await getSchedule()

    // schedule coloring
    days.forEach((day) => {
        if (schedule[day] !== undefined) {
            console.log('shedule for ' + day, schedule[day])
            for (let i = 0; i < 24; i++) {
                const id = i + '_' + day
                schedule[day].forEach(timePeriode => {
                    if (i >= timePeriode.start && i <= timePeriode.end) {
                        document.getElementById(id).innerHTML = timePeriode.zone
                        if (timePeriode.zone === 'Alpha') {
                            document.getElementById(id).style = 'background-color:cyan;'
                        } else if (timePeriode.zone === 'Beta') {
                            document.getElementById(id).style = 'background-color:green;'
                        }
                    }
                })
            }
        } else {
            console.log('no schedule for ' + day)
        }
    })
    outlineHour()
}

function outlineHour () {
    // outline the current day
    let dayNum = 0
    days.forEach((day) => {
        for (let i = 0; i < 24; i++) {
            const id = i + '_' + day
            if (d.getHours() === i && d.getDay() === dayNum) {
                // console.log(d.getHours(), d.getDay())
                console.log(id)
                document.getElementById(id).style = 'outline:inset;'
            }
        }
        dayNum++
    })
}

// const schedule = {
//     "Monday": [
//         {
//             start: 5,
//             end: 12,
//             zone: "Alpha"
//         },
//         {
//             start: 14,
//             end: 17,
//             zone: "Beta"
//         },
//     ],
//     "Tuesday": [
//         {
//             start: 1,
//             end: 5,
//             zone: "Beta"
//         },
//         {
//             start: 6,
//             end: 18,
//             zone: "Alpha"
//         }
//     ],
//     "Wednesday": [
//         {
//             start: 7,
//             end: 19,
//             zone: "Beta"
//         }
//     ],
//     "Thursday": [
//         {
//             start: 8,
//             end: 20,
//             zone: "Beta"
//         }
//     ],
//     "Friday": [
//         {
//             start: 9,
//             end: 21,
//             zone: "Beta"
//         }
//     ],
//     "Sunday": [
//         {
//             start: 10,
//             end: 24,
//             zone: "Beta"
//         }
//     ]
// }

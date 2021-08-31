import { sendCommand } from './comm';

window.start = function () {
    console.log('Sending Start');
    sendCommand('STATE_CHANGE', 'MOWING');
}
window.stop = function () {
    console.log('Sending Stop');
    sendCommand('STATE_CHANGE', 'IDLE');
}

window.setStateIcon = function (State) {
    switch (State) {
        case 'idle':
            return 'x';
            break;
        case 'docked':
            document.getElementById('stateDisplay').innerHTML = '<i class="fas fa - home"></i> Docked';
            break;
        case 'mowing':
            return 'x';
            break;
        case 'going_to_base':
            return 'x';
            break;
        case 'leaving_base':
            return 'x';
            break;
        case 'error':
            return 'x';
            break;
        case 'test':
            return 'x';
            break;
        default:
            document.getElementById('stateDisplay').innerHTML = "AAAA";
            break;
    }
}

console.log('Hello World!');

const days = [
    "Monday",
    "Tuesday",
    "Wednesday",
    "Thursday",
    "Friday",
    "Saterday",
    "Sunday"
]

const schedule = {
    monday :{
        start : "5:00",
        end : '17:00'
    }
}

// table header
days.forEach((day) => {
    document.getElementById("tableheader").innerHTML = document.getElementById("tableheader").innerHTML + `<th width="14.28%">${day}</th>`
})

// table body
for (let i = 1; i <= 24; i++) {
    let current = document.getElementById("tablebody").innerHTML
    let strDay = `<tr><td>${i}:00</td>`
    if(i == new Date().getHours()){
        console.log("current time");
        days.forEach((day) => {
            strDay = strDay + `<td class="has-events" rowspan="1">${day}</td>`
        })
    }
    else{
        
        days.forEach((day) => {
            strDay = strDay + `<td class="no-events" rowspan="1">${day}</td>`
        })
    }
   

    document.getElementById("tablebody").innerHTML = current + strDay + "</tr>";
}

//http://jsfiddle.net/dvirazulay/Lhe7C/
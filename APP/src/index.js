import { sendCommand } from './comm';

window.start = function () {
    console.log('Sending Start');
    sendCommand('STATE_CHANGE','MOWING');
}
window.stop = function () {
    console.log('Sending Stop');
    sendCommand('STATE_CHANGE','IDLE');
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
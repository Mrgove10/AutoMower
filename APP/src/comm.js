import config from './config.js';
import { connect } from 'mqtt/dist/mqtt';
console.log(config);

var client = connect('mqtt://' + config.mqttUrl)
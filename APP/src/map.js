import * as L from 'leaflet/dist/leaflet'

const mymap = L.map('mapid').setView([40.505, 5], 3)

L.tileLayer('https://{s}.tile.openstreetmap.org/{z}/{x}/{y}.png', {
    attribution: '&copy; <a href="https://www.openstreetmap.org/copyright">OpenStreetMap</a> contributors'
}).addTo(mymap)

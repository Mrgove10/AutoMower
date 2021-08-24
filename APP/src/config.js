const config = {
    mqttUrl: '192.168.1.101',
    mqttPort: '1883',
    mowerIP: '192.168.1.100'
}
/**
 * Get the curretn configuration
 * @returns curretn configuration
 */
export function getConfig() {
    return config;
}

export function saveConfig(){
    console.log('Saving config to Local Storage');
    localStorage.setItem('config', JSON.parse(config));
}

export function getStoredConfig(){
    var ls = localStorage.getItem(config);
    return JSON.parse(ls);
}
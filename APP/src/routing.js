/**
 * Change the page of the application
 * Since this is a single page app, the trick is to hide and unhide css part of the page
 * @param [home,schedule,advanced] page
 */
window.changepage = function (page) {
    console.log('changing page : ' + page)
    switch (page) {
        case 'home':
            document.getElementById('mainpage').style.display = 'flex'
            document.getElementById('schedule').style.display = 'none'
            document.getElementById('advanced').style.display = 'none'
            document.getElementById('scheduleEdit').style.display = 'none'
            break

        case 'schedule':
            document.getElementById('mainpage').style.display = 'none'
            document.getElementById('schedule').style.display = 'flex'
            document.getElementById('advanced').style.display = 'none'
            document.getElementById('scheduleEdit').style.display = 'none'

            break
        case 'scheduleEdit':
            document.getElementById('mainpage').style.display = 'none'
            document.getElementById('schedule').style.display = 'none'
            document.getElementById('advanced').style.display = 'none'
            document.getElementById('scheduleEdit').style.display = 'flex'

            break
        case 'advanced':
            document.getElementById('mainpage').style.display = 'none'
            document.getElementById('schedule').style.display = 'none'
            document.getElementById('advanced').style.display = 'flex'
            document.getElementById('scheduleEdit').style.display = 'none'
            break

        default:
            break
    }
}

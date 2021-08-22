window.changepage = function (page) {
    console.log('changing page : ' + page);
    switch (page) {
        case 'home':
            document.getElementById('mainpage').style.display = "flex";
            document.getElementById('schedule').style.display = "none";
            document.getElementById('advanced').style.display = "none";
            break;
        case 'schedule':
            document.getElementById('mainpage').style.display = "none";
            document.getElementById('schedule').style.display = "flex";
            document.getElementById('advanced').style.display = "none";
            break;
        case 'advanced':
            document.getElementById('mainpage').style.display = "none";
            document.getElementById('schedule').style.display = "none";
            document.getElementById('advanced').style.display = "flex";

            break;
        default:
            break;
    }
}
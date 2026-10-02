function onGameProcess() {
    //print("testRedirect.js is beeing called");

    for (let point = 1; point <= 9; point++) {
        const result = GET_TOUCH_POINT_STATE(point, 0);

        if (result) {
            print("GET_TOUCH_POINT_STATE(" + point + ") = " + result);
        }
    }
}
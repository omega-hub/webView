////////////////////////////////////////////////////////////////////////////////
requestAnimFrame = (function () {
    return window.requestAnimationFrame ||
    window.webkitRequestAnimationFrame ||
    window.mozRequestAnimationFrame ||
    window.oRequestAnimationFrame ||
    window.msRequestAnimationFrame ||
    function (/* function */callback, /* DOMElement */element) {
        // Since our requestAnimFrame is already in a loop in order to
        // control the preferred FPS, just call callback, not an interval
        callback();
    };
})();

////////////////////////////////////////////////////////////////////////////////
function frameLoop() {
    console.log("frame " + __omega.frame)
    if(typeof omega !== "undefined" && omega.frame > 5)
    {
        frame();
    }
    requestAnimFrame(frameLoop);
}

//requestAnimFrame(frameLoop);

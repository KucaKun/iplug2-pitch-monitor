function register_events() {
    canvas.addEventListener('mousedown', OnMouseDown)
    canvas.addEventListener('mouseup', OnMouseUp)
    canvas.addEventListener('mousemove', OnMouseMove)
    canvas.addEventListener('keydown', OnKeyDown)
    canvas.addEventListener('wheel', OnScroll)
    document.getElementById("yt-submit").addEventListener('click', OnDownloadSubmit)
}